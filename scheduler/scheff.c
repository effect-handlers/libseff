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

#include "atomic.h"
#include "scheff.h"

typedef struct task_t {
    seff_coroutine_t coroutine;
    scheff_poll_condition_t *poll_condition;
    void *poll_arg;
#ifndef NDEBUG
    int64_t id;
#endif
} task_t;

typedef struct worker_thread_t {
    struct scheff_t *scheduler;
    pthread_t thread;
    size_t worker_id;
    QUEUE(t) task_queue;

#ifndef NDEBUG
#define X(counter) int64_t counter;
    SCHEFF_DEBUG_COUNTERS
#undef X
#endif
} worker_thread_t;

/* This is the closest thing C gives us to an opaque type alias */
typedef struct scheff_waker_t {
    task_t *task;
} scheff_waker_t;

DEFINE_EFFECT(fork, 0, void, {
    seff_start_fun_t *fn;
    void *arg;
});
DEFINE_EFFECT(poll, 1, void, {
    scheff_poll_condition_t *poll_condition;
    void *poll_arg;
});

DEFINE_EFFECT(sleep, 2, void, {
    scheff_wakeup_manager_t *must_sleep;
    void *arg;
});

DEFINE_EFFECT(wake, 3, void, {
    size_t n_wakers;
    struct scheff_waker_t **wakers;
    bool resume;
});

void scheff_fork(seff_start_fun_t *fn, void *arg) { PERFORM(fork, fn, arg); }

void scheff_poll(scheff_poll_condition_t *cond, void *arg) { PERFORM(poll, cond, arg); }

void scheff_sleep(scheff_wakeup_manager_t *must_sleep, void *arg) {
    PERFORM(sleep, must_sleep, arg);
}
void scheff_wake(scheff_waker_t *wk, bool resume) { PERFORM(wake, 1, &wk, resume); }
void scheff_wake_all(size_t n_wakers, scheff_waker_t **wakers, bool resume) {
    PERFORM(wake, n_wakers, wakers, resume);
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
    debug(self->max_tasks = 0);
    debug(self->task_counter = 0);
    self->workers = aligned_alloc(64, n_workers * sizeof(worker_thread_t));
    if (!self->workers)
        return false;
    for (size_t i = 0; i < n_workers; i++) {
        self->workers[i].scheduler = self;
        self->workers[i].thread = 0;
        self->workers[i].worker_id = i;
#define X(counter) self->workers[i].counter = 0;
        debug(SCHEFF_DEBUG_COUNTERS);
#undef X
        QUEUE(init)(&self->workers[i].task_queue, INITIAL_QUEUE_LOG_SIZE);
    }
    return true;
}

bool scheff_schedule(scheff_t *self, seff_start_fun_t fn, void *arg) {
    task_t *task = (task_t *)malloc(sizeof(task_t));
    if (!task)
        return false;
    seff_coroutine_init_sized(&task->coroutine, fn, arg, STACK_SIZE);
    task->poll_condition = NULL;
    task->poll_arg = NULL;
    debug(task->id = self->task_counter++);
    self->remaining_tasks++;

    // TODO maybe load balancing and choose the worker?
    debug(self->workers[0].self_task_push++);
    QUEUE(push)(&self->workers[0].task_queue, task);
    return true;
}

task_t *scheff_try_get_task(worker_thread_t *self) {
    task_t *own_task = QUEUE(pop)(&self->task_queue);
    if (own_task != EMPTY && own_task != ABORT) {
        debug(self->self_task_pop++);
        return own_task;
    } else if (own_task == EMPTY) {
        debug(self->self_task_empty++);
    } else {
        assert(own_task == ABORT);
        debug(self->self_task_abort++);
    }

    size_t worker_id = self->worker_id;
    scheff_t *scheduler = self->scheduler;
    size_t n_workers = scheduler->n_workers;

    for (size_t i = 1; i < self->scheduler->n_workers; i++) {
        task_t *stolen_task =
            QUEUE(steal)(&scheduler->workers[(worker_id + i) % n_workers].task_queue);
        if (stolen_task != EMPTY && stolen_task != ABORT) {
            debug(self->stolen_task_ok++);
            return stolen_task;
        } else if (stolen_task == EMPTY) {
            debug(self->stolen_task_empty++);
        } else {
            assert(stolen_task == ABORT);
            debug(self->stolen_task_abort++);
        }
    }
    return NULL;
}

#define ENQUEUE(elt)                      \
    {                                     \
        debug(self->self_task_push += 1); \
        QUEUE(push)(task_queue, elt);     \
    }
#define ENQUEUE_PRIORITY(elt)                  \
    {                                          \
        debug(self->self_task_push += 1);      \
        QUEUE(priority_push)(task_queue, elt); \
    }
#define FINALIZE(task)                                                      \
    {                                                                       \
        debug({                                                             \
            int64_t n_tasks = RELAXED(load, remaining_tasks);               \
            int64_t max_tasks = RELAXED(load, &self->scheduler->max_tasks); \
            if (n_tasks > max_tasks) {                                      \
                RELAXED(store, &self->scheduler->max_tasks, n_tasks);       \
            }                                                               \
        });                                                                 \
        seff_coroutine_release(&task->coroutine);                           \
        free(task);                                                         \
        RELAXED(fetch_sub, remaining_tasks, 1);                             \
    }

void *scheff_worker_thread(void *_self) {
    worker_thread_t *self = (worker_thread_t *)(_self);
    QUEUE(t) *task_queue = &self->task_queue;
    _Atomic(int64_t) *remaining_tasks = &self->scheduler->remaining_tasks;

    // We loop forever. The actual termination condition is: inside the loop, if we've
    // failed to dequeue a task, we look at the remaining task counter and conditionally
    // break out
    while (true) {
        task_t *current_task = scheff_try_get_task(self);
        if (!current_task) {
            if (RELAXED(load, remaining_tasks) == 0) {
                break;
            }
            continue;
        }

        if (current_task->coroutine.state != PAUSED) {
            printf("Fatal error -- resuming finished coroutine!\n");
            exit(-1);
        }
        if (current_task->poll_condition && !current_task->poll_condition(current_task->poll_arg)) {
            debug(self->self_task_poll_fail += 1);
            ENQUEUE(current_task);
            continue;
        }
        current_task->poll_condition = NULL;

        seff_request_t request = seff_handle(&current_task->coroutine, NULL,
            HANDLES(fork) | HANDLES(poll) | HANDLES(sleep) | HANDLES(wake));
        switch (request.effect) {
            CASE_RETURN(request, {
                debug(self->return_requests++);
                FINALIZE(current_task);
                break;
            });
            CASE_EFFECT(request, fork, {
                debug(self->fork_requests++);

                RELAXED(fetch_add, remaining_tasks, 1);

                // TODO: duplicated with scheff_schedule
                task_t *new_task = malloc(sizeof(task_t));
                seff_coroutine_init_sized(
                    &new_task->coroutine, payload.fn, payload.arg, STACK_SIZE);
                new_task->poll_condition = NULL;
                new_task->poll_arg = NULL;
                debug(new_task->id = RELAXED(fetch_add, &self->scheduler->task_counter, 1));

                ENQUEUE(current_task);
                ENQUEUE_PRIORITY(new_task);
                break;
            });
            CASE_EFFECT(request, poll, {
                // TODO: why do we need poll, isn't sleep + event loop enough?
                debug(self->poll_requests++);

                static bool suppress_warning = false;
                if ((void *)QUEUE(push) == (void *)cl_queue_push && !suppress_warning) {
                    printf("WARNING! Using the suspend effect on Chase-Lev queues can lead to "
                           "livelocks\n");
                    suppress_warning = true;
                }
                current_task->poll_condition = payload.poll_condition;
                current_task->poll_arg = payload.poll_arg;
                ENQUEUE(current_task);
                break;
            });
            CASE_EFFECT(request, sleep, {
                debug(self->sleep_requests++);

                scheff_waker_t *waker = (scheff_waker_t *)current_task;
                if (!payload.must_sleep(waker, payload.arg)) {
                    ENQUEUE(current_task);
                }

                break;
            });
            CASE_EFFECT(request, wake, {
                debug(self->wake_requests++);

                for (size_t i = 0; i < payload.n_wakers; i++) {
                    task_t *task = (task_t *)payload.wakers[i];
                    // TODO: Only the last one is priority, right?
                    ENQUEUE_PRIORITY(task);
                }

                if (payload.resume) {
                    ENQUEUE(current_task);
                } else {
                    FINALIZE(current_task);
                }
                break;
            });
        default:
            assert(false);
        }
    }

    return NULL;
}
#undef FINALIZE
#undef ENQUEUE_PRIORITY
#undef ENQUEUE

void scheff_run(scheff_t *scheduler) {
    for (size_t i = 1; i < scheduler->n_workers; i++) {
        pthread_create(
            &scheduler->workers[i].thread, NULL, scheff_worker_thread, &scheduler->workers[i]);
    }
    // Become the first worker
    scheduler->workers[0].thread = pthread_self();
    scheff_worker_thread(&scheduler->workers[0]);

    for (size_t i = 1; i < scheduler->n_workers; i++) {
        void *ignore_return;
        pthread_join(scheduler->workers[i].thread, &ignore_return);
    }
}

void scheff_print_stats(scheff_t *scheduler) {
#ifndef NDEBUG
    int64_t total_contention = 0;
    for (size_t i = 0; i < scheduler->n_workers; i++) {
        printf("Worker %lu:\n", i);
        total_contention += scheduler->workers[i].self_task_abort;
        total_contention += scheduler->workers[i].self_task_empty;
        total_contention += scheduler->workers[i].stolen_task_abort;
        total_contention += scheduler->workers[i].stolen_task_empty;
        total_contention += scheduler->workers[i].spinlock_fails;

#define X(counter) printf("\t" #counter ": %ld\n", scheduler->workers[i].counter);
        SCHEFF_DEBUG_COUNTERS;
#undef X
    }

    printf("Relative contention %lf\n", ((double)total_contention) / scheduler->n_workers);
    printf("Max concurrent tasks %ld\n", scheduler->max_tasks);
#endif
}
