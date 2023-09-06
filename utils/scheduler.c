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

#include <assert.h>
#include <poll.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/poll.h>

#include "scheduler.h"
#include "seff.h"
#include <stdatomic.h>

typedef struct {
    seff_coroutine_t *cont;
    int awaiting_fd;
    short awaiting_events;
    short replied_events;
} task_t;

#include "queue_generic.h"
#if defined SCHEDULER_POLICY_WORK_SPILLING
#define POLICY_WORK_SPILLING 1
#define POLICY_WORK_STEALING 0
#define POLICY_SIMPLE 0
#include "queue_np1c.c"
#elif defined SCHEDULER_POLICY_WORK_STEALING
#define POLICY_WORK_SPILLING 0
#define POLICY_WORK_STEALING 1
#define POLICY_SIMPLE 0
#include "queue_1pnc.c"
#else
#define POLICY_WORK_SPILLING 0
#define POLICY_WORK_STEALING 0
#define POLICY_SIMPLE 1
#include "queue_1p1c.c"
#endif

struct scheduler_t;

typedef struct {
    struct scheduler_t *scheduler;
    int64_t thread_id;
    pthread_t thread;
    queue_t task_queue;
    int64_t next_fork_thread;
} scheduler_thread_t;

typedef struct scheduler_t {
    size_t n_threads;
    scheduler_thread_t *threads;
    _Atomic int64_t remaining_tasks;
} scheduler_t;

bool scheduler_init(scheduler_t *self, size_t n_threads, size_t max_task_queue_size) {
    pthread_mutex_init(&puts_mutex, NULL);

    self->n_threads = n_threads;
    self->threads = calloc(n_threads, sizeof(scheduler_thread_t));
    if (!self->threads) {
        return false;
    }

    for (size_t i = 0; i < n_threads; i++) {
        self->threads[i].scheduler = self;
        self->threads[i].thread_id = i;
        self->threads[i].next_fork_thread = i;
        if (!queue_init(&self->threads[i].task_queue, max_task_queue_size)) {
            while (i > 0) {
                i--;
                queue_destroy(&self->threads[i].task_queue);
            }
            free(self->threads);
            return false;
        }
    }
    atomic_store(&self->remaining_tasks, 0);
    return true;
}
void scheduler_schedule_task(scheduler_t *scheduler, task_t task, int thread_id) {
    atomic_fetch_add_explicit(&scheduler->remaining_tasks, 1, memory_order_relaxed);
    int64_t target_thread = scheduler->threads[thread_id].next_fork_thread;
#ifdef SCHEDULER_POLICY_WORK_SPILLING
    scheduler->threads[thread_id].next_fork_thread = (target_thread + 1) % (scheduler->n_threads - 1);
#endif
    queue_t *thread_queue = &scheduler->threads[target_thread].task_queue;
    while (!queue_enqueue(thread_queue, task)) {
        // Active wait
#ifndef NDEBUG
        threadsafe_puts("FAILED TO ENQUEUE");
#endif
    }
}
void scheduler_schedule(scheduler_t *scheduler, seff_start_fun_t *fn, void *arg, int thread_id) {
    seff_coroutine_t *coroutine = seff_coroutine_new(fn, arg);
    scheduler_schedule_task(scheduler, (task_t){coroutine, -1, 0}, thread_id);
}

#ifndef NDEBUG
#define thread_report(msg, ...) \
    threadsafe_printf("[worker thread %ld]: " msg, thread_id, ##__VA_ARGS__)
#else
#define thread_report(msg, ...)
#endif
void handle_request(scheduler_thread_t *self, task_t task) {
    scheduler_t *scheduler = self->scheduler;
    seff_eff_t *request = (seff_eff_t *)seff_handle(task.cont,
        (void *)(uintptr_t)task.replied_events, HANDLES(yield) | HANDLES(fork) | HANDLES(await));
    task.replied_events = 0;
    if (task.cont->state == FINISHED) {
        seff_coroutine_delete(task.cont);
        atomic_fetch_sub_explicit(&scheduler->remaining_tasks, 1, memory_order_relaxed);
    } else {
        switch (request->id) {
            CASE_EFFECT(request, yield, {
                (void)payload;
                break;
            });
            CASE_EFFECT(request, await, {
                task.awaiting_fd = payload.fd;
                task.awaiting_events = (short)payload.events;
                break;
            });
            CASE_EFFECT(request, fork, {
                scheduler_schedule(scheduler, payload.function, payload.argument, self->thread_id);
                break;
            });
        default:
            assert(false);
        }
        queue_t *task_queue = &self->task_queue;

        int64_t thread_id = self->thread_id;
        (void)thread_id;
        while (!queue_enqueue(task_queue, task)) {
            thread_report("failed to enqueue task");
        }
    }
}

void *worker_thread(void *args) {
    scheduler_thread_t *self = (scheduler_thread_t *)args;
    scheduler_t *scheduler = self->scheduler;
    int64_t thread_id = self->thread_id;
    (void)thread_id;
    queue_t *task_queue = &self->task_queue;
    thread_report("spawning\n");

#ifdef SCHEDULER_POLICY_WORK_STEALING
    const size_t n_threads = scheduler->n_threads;
#endif
    while (atomic_load_explicit(&scheduler->remaining_tasks, memory_order_relaxed) > 0) {
        task_t task = queue_dequeue(task_queue);
#ifdef SCHEDULER_POLICY_WORK_STEALING
        if (!task.cont && queue_size(task_queue) > 0) {
            // Do not try to steal a task unless this thread's queue is empty.
            // This makes live-locks less bad.
            continue;
        }
        for (size_t i = 1; !task.cont && i < n_threads; i++) {
            // Try to steal a task from another thread's queue
            int64_t target_thread = (thread_id + 1) % n_threads;
            task = queue_dequeue(&scheduler->threads[target_thread].task_queue);
        }
#endif
        // If none of the threads had any available tasks, this might mean all
        // the tasks were completed by another thread after we checked for
        // `remaining_tasks.load() > 0`, so loop again
        if (!task.cont)
            continue;

        // If the task was an `await` task, we poll the underlying file
        // descriptors and only resume the task if they're ready, otherwise
        // we re-enqueue the task
        if (task.awaiting_fd >= 0) {
            struct pollfd polls;
            polls.events = task.awaiting_events;
            polls.fd = task.awaiting_fd;
            if (!poll(&polls, 1, 0)) {
                while (!queue_enqueue(task_queue, task)) {
                    thread_report("failed to enqueue task");
                }
                continue;
            } else {
                // Task is no longer awaiting for a successful poll
                task.replied_events = polls.revents;
                task.awaiting_events = 0;
                task.awaiting_fd = -1;
            }
        }
        handle_request(self, task);
    }
    thread_report("exiting\n");
    return NULL;
}
void scheduler_start(scheduler_t *scheduler) {
    for (size_t i = 0; i < scheduler->n_threads; i++) {
        pthread_create(&scheduler->threads[i].thread, NULL, worker_thread, &scheduler->threads[i]);
    }
}
bool scheduler_finished(scheduler_t *scheduler) {
    return atomic_load(&scheduler->remaining_tasks) == 0;
}
void scheduler_join(scheduler_t *scheduler) {
    for (size_t i = 0; i < scheduler->n_threads; i++) {
        void *ret;
        pthread_join(scheduler->threads[i].thread, &ret);
    }
}
void scheduler_destroy(scheduler_t *scheduler) {
    for (size_t i = 0; i < scheduler->n_threads; i++) {
        queue_destroy(&scheduler->threads[i].task_queue);
    }
    free(scheduler->threads);
}
