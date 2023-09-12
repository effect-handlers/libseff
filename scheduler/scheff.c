/*
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
 *
 * libseff is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	    http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 */

#include "scheff.h"

#define RELAXED(op, ...) atomic_##op##_explicit(__VA_ARGS__, memory_order_relaxed)
#define ACQUIRE(op, ...) atomic_##op##_explicit(__VA_ARGS__, memory_order_acquire)
#define RELEASE(op, ...) atomic_##op##_explicit(__VA_ARGS__, memory_order_release)

DEFINE_EFFECT(fork, 0, void, {
    seff_start_fun_t *fn;
    void *arg;
});
DEFINE_EFFECT(await, 1, bool, { future_t *fut; });
DEFINE_EFFECT(notify, 2, void, { future_t *fut; });

bool try_lock(future_t *fut) {
    bool expected = false;
    return atomic_compare_exchange_weak_explicit(
        &fut->blocked, &expected, true, memory_order_acquire, memory_order_relaxed);
}

bool unlock(future_t *fut) {
    return atomic_exchange_explicit(&fut->blocked, false, memory_order_release);
}

#define SPINLOCK(future, block)            \
    {                                      \
        while (!try_lock(future)) {        \
            debug(self->spinlock_fails++); \
        }                                  \
        block unlock(future);              \
    }

void scheff_fork(seff_start_fun_t *fn, void *arg) { PERFORM(fork, fn, arg); }
void *scheff_await(future_t *fut) {
    future_state_t state = WRITING;
    while (state == WRITING) {
        state = RELAXED(load, &fut->state);
    }
    switch (state) {
    case READY:
        return fut->result;
    case WAITING:
        PERFORM(await, fut);
        return fut->result;
    case CANCELLED:
        return NULL;
    default:
        // Should not happen
        return NULL;
    }
}
bool scheff_fulfill(future_t *fut, void *result) {
    future_state_t expected = WAITING;

    if (atomic_compare_exchange_strong_explicit(
            &fut->state, &expected, WRITING, memory_order_acquire, memory_order_relaxed)) {
        fut->result = result;
        RELEASE(store, &fut->state, READY);
        PERFORM(notify, fut);
        return true;
    } else {
        return false;
    }
}

#define STACK_SIZE 512
#define INITIAL_QUEUE_LOG_SIZE 3

#ifndef NDEBUG
#define debug(block) block
#else
#define debug(block)
#endif

bool scheff_init(scheff_t *self, size_t n_workers) {
    assert(n_workers > 0);
    self->n_workers = n_workers;
    self->remaining_tasks = 0;
    self->workers = malloc(n_workers * sizeof(worker_thread_t));
    if (!self->workers)
        return false;
    for (size_t i = 0; i < n_workers; i++) {
        self->workers[i].scheduler = self;
        self->workers[i].thread = 0;
        self->workers[i].worker_id = i;
        debug({
            self->workers[i].self_task_abort = 0;
            self->workers[i].self_task_empty = 0;
            self->workers[i].stolen_task_abort = 0;
            self->workers[i].stolen_task_empty = 0;
            self->workers[i].spinlock_fails = 0;
        });
        cl_queue_init(&self->workers[i].task_queue, INITIAL_QUEUE_LOG_SIZE);
    }
    return true;
}

void scheff_schedule(scheff_t *self, seff_start_fun_t fn, void *arg) {
    task_t *task = (task_t *)malloc(sizeof(task_t));
    seff_coroutine_init_sized(&task->coroutine, fn, arg, STACK_SIZE);
    self->remaining_tasks++;

    debug(self->workers[0].self_task_push++);
    cl_queue_push(&self->workers[0].task_queue, task);
}

task_t *try_get_task(worker_thread_t *self) {
    task_t *own_task = cl_queue_pop(&self->task_queue);
    if (own_task != EMPTY) {
        debug(self->self_task_pop++);
        return own_task;
    } else if (own_task == EMPTY) {
        debug(self->self_task_empty++);
    } else {
        debug(self->self_task_abort++);
    }

    size_t worker_id = self->worker_id;
    scheff_t *scheduler = self->scheduler;
    size_t n_workers = scheduler->n_workers;

    for (size_t i = 1; i < self->scheduler->n_workers; i++) {
        task_t *stolen_task =
            cl_queue_steal(&scheduler->workers[(worker_id + i) % n_workers].task_queue);
        if (stolen_task != EMPTY && stolen_task != ABORT) {
            debug(self->stolen_task_ok++);
            return stolen_task;
        } else if (stolen_task == EMPTY) {
            debug(self->stolen_task_empty++);
        } else {
            debug(self->stolen_task_abort++);
        }
    }
    return NULL;
}

void *worker_thread(void *_self) {
    worker_thread_t *self = (worker_thread_t *)(_self);
    cl_queue_t *task_queue = &self->task_queue;
    _Atomic(int64_t) *remaining_tasks = &self->scheduler->remaining_tasks;

    // We loop forever. The actual termination condition is: inside the loop, if we've
    // failed to dequeue a task, we look at the remaining task counter and conditionally
    // break out
    while (true) {
        task_t *current_task = try_get_task(self);
        if (!current_task) {
            if (RELAXED(load, remaining_tasks) == 0) {
                break;
            }
            continue;
        }

        if (current_task->coroutine.state != PAUSED) {
            printf("WTF!\n");
        }
        seff_eff_t *request = (seff_eff_t *)seff_handle(
            &current_task->coroutine, NULL, HANDLES(fork) | HANDLES(await) | HANDLES(notify));
        if (current_task->coroutine.state == FINISHED) {
            debug(self->return_requests++);

            seff_coroutine_release(&current_task->coroutine);
            free(current_task);
            RELAXED(fetch_sub, remaining_tasks, 1);
        } else {
            switch (request->id) {
                CASE_EFFECT(request, await, {
                    debug(self->await_requests++);

                    SPINLOCK(payload.fut, {
                        if (payload.fut->waiter) {
                            exit(-1);
                        }
                        if (payload.fut->state == READY) {
                            debug(self->self_task_push += 1);
                            cl_queue_push(task_queue, current_task);
                        } else {
                            payload.fut->waiter = current_task;
                        }
                    });
                    break;
                });
                CASE_EFFECT(request, fork, {
                    debug(self->fork_requests++);
                    RELAXED(fetch_add, remaining_tasks, 1);

                    // TODO: duplicated with scheff_schedule
                    task_t *task = malloc(sizeof(task_t));
                    seff_coroutine_init_sized(
                        &task->coroutine, payload.fn, payload.arg, STACK_SIZE);
                    debug(self->self_task_push += 2);
                    cl_queue_push(task_queue, task);
                    cl_queue_push(task_queue, current_task);
                    break;
                });
                CASE_EFFECT(request, notify, {
                    SPINLOCK(payload.fut, {
                        task_t *waiter = payload.fut->waiter;
                        payload.fut->waiter = NULL;
                        if (waiter) {
                            debug(self->self_task_push += 1);
                            cl_queue_priority_push(task_queue, waiter);
                        }
                    });
                    debug(self->self_task_push += 1);
                    cl_queue_push(task_queue, current_task);
                    break;
                });
            default:
                assert(false);
            }
        }
    }

    return NULL;
}

void scheff_run(scheff_t *scheduler) {
    for (size_t i = 1; i < scheduler->n_workers; i++) {
        pthread_create(&scheduler->workers[i].thread, NULL, worker_thread, &scheduler->workers[i]);
    }
    // Become the first worker
    scheduler->workers[0].thread = pthread_self();
    worker_thread(&scheduler->workers[0]);

    for (size_t i = 1; i < scheduler->n_workers; i++) {
        void *ignore_return;
        pthread_join(scheduler->workers[i].thread, &ignore_return);
    }
}

void scheff_print_stats(scheff_t *scheduler) {
    int64_t total_contention = 0;
    for (size_t i = 0; i < scheduler->n_workers; i++) {
        printf("Worker %lu:\n", i);
        total_contention += scheduler->workers[i].self_task_abort;
        total_contention += scheduler->workers[i].self_task_empty;
        total_contention += scheduler->workers[i].stolen_task_abort;
        total_contention += scheduler->workers[i].stolen_task_empty;
        total_contention += scheduler->workers[i].spinlock_fails;
        printf("\tself_task_push: %ld\n\n", scheduler->workers[i].self_task_push);

        printf("\tself_task_pop: %ld\n", scheduler->workers[i].self_task_pop);
        printf("\tself_task_abort: %ld\n", scheduler->workers[i].self_task_abort);
        printf("\tself_task_empty: %ld\n\n", scheduler->workers[i].self_task_empty);

        printf("\tstolen_task_ok: %ld\n", scheduler->workers[i].stolen_task_ok);
        printf("\tstolen_task_abort: %ld\n", scheduler->workers[i].stolen_task_abort);
        printf("\tstolen_task_empty: %ld\n", scheduler->workers[i].stolen_task_empty);
        printf("\tspinlock_fails: %ld\n\n", scheduler->workers[i].spinlock_fails);

        printf("\tfork_requests: %ld\n", scheduler->workers[i].fork_requests);
        printf("\tawait_requests: %ld\n", scheduler->workers[i].await_requests);
        printf("\tnotify_requests: %ld\n", scheduler->workers[i].notify_requests);
        printf("\treturn_requests: %ld\n", scheduler->workers[i].return_requests);
    }

    printf("Relative contention %lf\n", ((double)total_contention) / scheduler->n_workers);
}
