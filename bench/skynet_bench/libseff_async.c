#include "seff.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cl_queue.h"

#undef NDEBUG

#define STACK_SIZE 512

typedef enum { READY, WAITING } future_state_t;

struct task_t;
typedef struct task_t *taskptr;
typedef struct {
    _Atomic bool blocked;
    future_state_t state;
    int64_t result;
    struct task_t *waiter;
} future_t;

bool try_lock(future_t *fut) {
    bool expected = false;
    return atomic_compare_exchange_weak_explicit(
        &fut->blocked, &expected, true, memory_order_relaxed, memory_order_relaxed);
}

bool unlock(future_t *fut) {
    return atomic_exchange_explicit(&fut->blocked, false, memory_order_relaxed);
}

#ifndef NDEBUG
#define SPINLOCK(future, block)     \
    {                               \
        while (!try_lock(future)) { \
            self->spinlock_fails++; \
        }                           \
        block unlock(future);       \
    }
#else
#define SPINLOCK(future, block)     \
    {                               \
        while (!try_lock(future)) { \
        }                           \
        block unlock(future);       \
    }
#endif

typedef struct task_t {
    seff_coroutine_t *coroutine;
    future_t *waiting;
    future_t *promise;
} task_t;

DEFINE_EFFECT(async, 2, void, {
    seff_start_fun_t *fn;
    void *arg;
    future_t *promise;
});
DEFINE_EFFECT(await, 3, int64_t, { future_t *fut; });

struct async_scheduler_t;
typedef struct worker_thread_t {
    struct async_scheduler_t *scheduler;
    pthread_t thread;
    size_t worker_id;
    queue_t task_queue;

#ifndef NDEBUG
    int64_t self_task_push;

    int64_t self_task_pop;
    int64_t self_task_abort;
    int64_t self_task_empty;

    int64_t stolen_task_ok;
    int64_t stolen_task_abort;
    int64_t stolen_task_empty;

    int64_t spinlock_fails;

    int64_t async_requests;
    int64_t await_requests;
    int64_t return_requests;
#endif
} worker_thread_t;

typedef struct async_scheduler_t {
    size_t n_workers;
    worker_thread_t *workers;
    _Atomic int64_t remaining_tasks;
} async_scheduler_t;

bool async_scheduler_init(async_scheduler_t *self, size_t n_workers) {
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
#ifndef NDEBUG
        self->workers[i].self_task_abort = 0;
        self->workers[i].self_task_empty = 0;
        self->workers[i].stolen_task_abort = 0;
        self->workers[i].stolen_task_empty = 0;
        self->workers[i].spinlock_fails = 0;
#endif
        cl_queue_init(&self->workers[i].task_queue, 3);
    }
    return true;
}

void async_schedule(
    async_scheduler_t *self, size_t worker_id, seff_start_fun_t fn, void *arg, future_t *promise) {
    seff_coroutine_t *k = seff_coroutine_new_sized(fn, arg, STACK_SIZE);
    task_t *task = (task_t *)malloc(sizeof(task_t));
    task->coroutine = k;
    task->waiting = NULL;
    task->promise = promise;
    self->remaining_tasks++;

    self->workers[0].self_task_push++;
    cl_queue_push(&self->workers[0].task_queue, task);
}

task_t *try_get_task(worker_thread_t *self) {
    task_t *own_task = cl_queue_pop(&self->task_queue);
    if (own_task != EMPTY && own_task != ABORT) {
        self->self_task_pop++;
        return own_task;
    }
#ifndef NDEBUG
    else if (own_task == EMPTY) {
        self->self_task_empty++;
    } else {
        self->self_task_abort++;
    }
#endif

    size_t worker_id = self->worker_id;
    async_scheduler_t *scheduler = self->scheduler;
    size_t n_workers = scheduler->n_workers;

    for (size_t i = 1; i < self->scheduler->n_workers; i++) {
        task_t *stolen_task =
            cl_queue_steal(&scheduler->workers[(worker_id + i) % n_workers].task_queue);
        if (stolen_task != EMPTY && stolen_task != ABORT) {
            self->stolen_task_ok++;
            return stolen_task;
        }
#ifndef NDEBUG
        else if (stolen_task == EMPTY) {
            self->stolen_task_empty++;
        } else {
            self->stolen_task_abort++;
        }
#endif
    }
    return NULL;
}

void *worker_thread(void *_self) {
    worker_thread_t *self = (worker_thread_t *)(_self);
    _Atomic int64_t *remaining_tasks = &self->scheduler->remaining_tasks;

    // We loop forever. The actual termination condition is: inside the loop, if we've
    // failed to dequeue a task, we look at the remaining task counter and conditionally
    // break out
    while (true) {
        task_t *current_task = try_get_task(self);
        if (!current_task) {
            if (atomic_load_explicit(remaining_tasks, memory_order_relaxed) == 0) {
                break;
            }
            continue;
        }

        void *coroutine_arg;
        if (current_task->waiting != NULL) {
            assert(current_task->waiting->state != WAITING);
            coroutine_arg = (void *)current_task->waiting->result;
            current_task->waiting->waiter = NULL;
            current_task->waiting = NULL;
        } else {
            coroutine_arg = NULL;
        }

        seff_eff_t *request = (seff_eff_t *)seff_handle(
            current_task->coroutine, coroutine_arg, HANDLES(async) | HANDLES(await));
        if (current_task->coroutine->state == FINISHED) {
            self->return_requests++;
            future_t *promise = current_task->promise;

            /* Spin until the future can be acquired */
            SPINLOCK(promise, {
                promise->result = (int64_t)(request);
                promise->state = READY;

                task_t *waiter = promise->waiter;
                if (waiter != NULL) {
                    self->self_task_push++;
                    cl_queue_push(&self->task_queue, waiter);
                }
            });

            seff_coroutine_delete(current_task->coroutine);
            free(current_task);
            atomic_fetch_sub_explicit(remaining_tasks, 1, memory_order_relaxed);
        } else {
            switch (request->id) {
                CASE_EFFECT(request, await, {
                    self->await_requests++;
                    future_t *awaited = payload.fut;
                    SPINLOCK(awaited, {
                        current_task->waiting = payload.fut;
                        if (payload.fut->state == READY) {
                            self->self_task_push++;
                            cl_queue_push(&self->task_queue, current_task);
                        } else {
                            payload.fut->waiter = current_task;
                        }
                    });
                    break;
                });
                CASE_EFFECT(request, async, {
                    self->async_requests++;
                    atomic_fetch_add_explicit(remaining_tasks, 1, memory_order_relaxed);

                    payload.promise->state = WAITING;
                    payload.promise->waiter = NULL;
                    payload.promise->blocked = false;

                    seff_coroutine_t *k =
                        seff_coroutine_new_sized(payload.fn, payload.arg, STACK_SIZE);
                    task_t *new_task = (task_t *)malloc(sizeof(task_t));
                    new_task->coroutine = k;
                    new_task->promise = payload.promise;
                    new_task->waiting = NULL;

                    self->self_task_push += 2;
                    cl_queue_push(&self->task_queue, new_task);
                    cl_queue_push(&self->task_queue, current_task);
                    break;
                });
            default:
                assert(false);
            }
        }
    }

    return NULL;
}

void async_scheduler_run(async_scheduler_t *scheduler) {
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

// First node id of the last layer
int64_t last_layer = 1;
void *skynet(seff_coroutine_t *self, void *_arg) {
    int64_t num = (int64_t)_arg;

    if (num >= last_layer) {
        return (void *)(uintptr_t)(num - last_layer);
    } else {
        int64_t sum = 0;

        num *= 10;
        future_t futures[10];
        for (size_t i = 0; i < 10; i++) {
            PERFORM(async, skynet, (void *)(num + i), &futures[i]);
        }
        for (size_t i = 0; i < 10; i++) {
            int64_t partial_result = PERFORM(await, &futures[i]);
            sum += partial_result;
        }
        return (void *)(uintptr_t)sum;
    }
}

void print_usage(char *self) {
    printf("Usage: %s [--depth M] [--threads N]\n", self);
    exit(-1);
}

int main(int argc, char **argv) {
    int n_workers = 8;
    int depth = 7;

    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--threads") == 0) {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
            }
            int res = sscanf(argv[i + 1], "%d", &n_workers);
            if (res <= 0 || n_workers <= 0) {
                print_usage(argv[0]);
            }
            i++;
        } else if (strcmp(argv[i], "--depth") == 0) {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
            }
            int res = sscanf(argv[i + 1], "%d", &depth);
            if (res <= 0 || depth <= 0) {
                print_usage(argv[0]);
            }
            i++;
        } else {
            print_usage(argv[0]);
        }
    }

    for (int i = 1; i < depth; i++) {
        last_layer *= 10;
    }

    async_scheduler_t scheduler;
    async_scheduler_init(&scheduler, n_workers);

    future_t final_result = (future_t){false, WAITING, 0, NULL};
    async_schedule(&scheduler, 0, skynet, (void *)(uintptr_t)1, &final_result);

    async_scheduler_run(&scheduler);

    printf("Total: %ld\n", final_result.result);

#ifndef NDEBUG
    int64_t total_contention = 0;
    for (size_t i = 0; i < n_workers; i++) {
        printf("Worker %lu:\n", i);
        total_contention += scheduler.workers[i].self_task_abort;
        total_contention += scheduler.workers[i].self_task_empty;
        total_contention += scheduler.workers[i].stolen_task_abort;
        total_contention += scheduler.workers[i].stolen_task_empty;
        total_contention += scheduler.workers[i].spinlock_fails;
        printf("\tself_task_push: %ld\n\n", scheduler.workers[i].self_task_push);

        printf("\tself_task_pop: %ld\n", scheduler.workers[i].self_task_pop);
        printf("\tself_task_abort: %ld\n", scheduler.workers[i].self_task_abort);
        printf("\tself_task_empty: %ld\n\n", scheduler.workers[i].self_task_empty);

        printf("\tstolen_task_ok: %ld\n", scheduler.workers[i].stolen_task_ok);
        printf("\tstolen_task_abort: %ld\n", scheduler.workers[i].stolen_task_abort);
        printf("\tstolen_task_empty: %ld\n", scheduler.workers[i].stolen_task_empty);
        printf("\tspinlock_fails: %ld\n\n", scheduler.workers[i].spinlock_fails);

        printf("\tasync_requests: %ld\n", scheduler.workers[i].async_requests);
        printf("\tawait_requests: %ld\n", scheduler.workers[i].await_requests);
        printf("\treturn_requests: %ld\n", scheduler.workers[i].return_requests);
    }

    printf("Relative contention %lf\n", ((double)total_contention) / n_workers);
#endif
    return 0;
}
