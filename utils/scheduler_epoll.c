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

typedef struct _task_t {
    seff_coroutine_t *cont;
    int watched_fd;
    uint32_t watched_events;
    uint32_t ready_events;
} _task_t;

typedef _task_t *task_t;
#include "queue_np1c.c"

struct scheduler_t;

typedef struct {
    struct scheduler_t *scheduler;
    int64_t thread_id;
    pthread_t thread;
    queue_t task_queue;
    int64_t next_fork_thread;
    int epoll_fd;
} scheduler_thread_t;

typedef struct scheduler_t {
    size_t n_threads;
    scheduler_thread_t *threads;
    _Atomic int64_t remaining_tasks;
} scheduler_t;

#ifndef NDEBUG
#define thread_report(msg, ...) \
    threadsafe_printf("[worker thread %ld]: " msg, thread_id, ##__VA_ARGS__)
#else
#define thread_report(msg, ...)
#endif

void scheduler_schedule_task(scheduler_t *scheduler, task_t task, int64_t thread_id) {
    atomic_fetch_add(&scheduler->remaining_tasks, 1);
    int64_t target_thread = scheduler->threads[thread_id].next_fork_thread;
    scheduler->threads[thread_id].next_fork_thread = (target_thread + 1) % scheduler->n_threads;
    queue_t *thread_queue = &scheduler->threads[target_thread].task_queue;
    while (!queue_enqueue(thread_queue, task)) {
        // Active wait
#ifndef NDEBUG
        threadsafe_puts("FAILED TO ENQUEUE");
#endif
    }
}

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

// #define MAX_TASKS 200000
// _Thread_local size_t next_task = 0;
// _Thread_local _task_t tasks[MAX_TASKS];
void scheduler_schedule(scheduler_t *scheduler, seff_start_fun_t fn, void *arg, int64_t thread_id) {
    seff_coroutine_t *k = seff_coroutine_new(fn, arg);
    task_t task = malloc(sizeof(_task_t));
    // task_t task = &tasks[next_task];
    // next_task = (next_task + 1) % MAX_TASKS;
    task->watched_fd = -1;
    task->watched_events = 0;
    task->ready_events = 0;
    task->cont = k;
    scheduler_schedule_task(scheduler, task, thread_id);
}

bool can_run(task_t task) {
    return (task->watched_events == 0 || (task->watched_events & task->ready_events));
}

void run_task(scheduler_thread_t *self, task_t task) {
    int64_t thread_id = self->thread_id;
    (void)thread_id;
    scheduler_t *scheduler = self->scheduler;
    void *task_input = (void *)(uintptr_t)task->ready_events;
    thread_report("running task\n");

    seff_eff_t *request = (seff_eff_t *)seff_handle(
        task->cont, task_input, HANDLES(yield) | HANDLES(fork) | HANDLES(await));
    if (task->cont->state == FINISHED) {
        seff_coroutine_delete(task->cont);
        atomic_fetch_sub_explicit(&scheduler->remaining_tasks, 1, memory_order_relaxed);
        if (task->watched_fd != -1)
            epoll_ctl(self->epoll_fd, EPOLL_CTL_DEL, task->watched_fd, NULL);
        free(task);
        return;
    }
    queue_t *task_queue = &self->task_queue;
    switch (request->id) {
        CASE_EFFECT(request, yield, {
            (void)payload;
            while (!queue_enqueue(task_queue, task)) {
                thread_report("failed to enqueue task\n");
            }
            break;
        });
        CASE_EFFECT(request, await, {
            task->watched_events = payload.events;
            task->ready_events = 0;
            struct epoll_event event;
            event.events = task->watched_events;
            event.data.ptr = task;
            int result;
            (void)result;
            if (task->watched_fd == -1) {
                task->watched_fd = payload.fd;
                result = epoll_ctl(self->epoll_fd, EPOLL_CTL_ADD, task->watched_fd, &event);
            } else if (task->watched_fd == payload.fd && task->watched_events != payload.events) {
                result = epoll_ctl(self->epoll_fd, EPOLL_CTL_MOD, task->watched_fd, &event);
            } else if (task->watched_fd == payload.fd) {
                result = 0;
            } else {
                assert(false);
            }
            assert(result == 0);
            break;
        });
        CASE_EFFECT(request, fork, {
            scheduler_schedule(scheduler, payload.function, payload.argument, self->thread_id);
            while (!queue_enqueue(task_queue, task)) {
                thread_report("failed to enqueue task\n");
            }
            break;
        });
    default:
        assert(false);
    }
    return;
}

#define MAX_EVENTS 512
void *worker_thread(void *args) {
    scheduler_thread_t *self = (scheduler_thread_t *)args;
    scheduler_t *scheduler = self->scheduler;
    int64_t thread_id = self->thread_id;
    (void)thread_id;
    queue_t *task_queue = &self->task_queue;
    thread_report("spawning\n");

    self->epoll_fd = epoll_create1(0);
    struct epoll_event events[MAX_EVENTS];
    while (atomic_load_explicit(&scheduler->remaining_tasks, memory_order_relaxed) > 0) {
        task_t task = queue_dequeue(task_queue);
        if (task) {
            // There's a valid task
            run_task(self, task);
            task->ready_events = 0;
        } else {
            // The queue is empty, wait and add to queue
            // Instead of 1 we should reserve spaces on the queue, epoll_wait
            // on the number of spaces reserved, and then add the tasks
            // that are ready, and release spaces not used
            int n_events = epoll_wait(self->epoll_fd, events, 1, 0);
            if (n_events < 0) {
                threadsafe_printf("epoll error\n");
                break;
            }
            for (size_t i = 0; i < n_events; i++) {
                task_t watcher = (task_t)events[i].data.ptr;
                watcher->ready_events |= events[i].events;
                if (can_run(watcher)) {
                    epoll_ctl(self->epoll_fd, EPOLL_CTL_DEL, watcher->watched_fd, NULL);
                    watcher->watched_fd = -1;
                    while (!queue_enqueue(task_queue, watcher)) {
                        threadsafe_printf("failed to enqueue task\n");
                    }
                }
            }
        }
    }

    threadsafe_printf("exiting with %ld remaining tasks\n",
        atomic_load_explicit(&scheduler->remaining_tasks, memory_order_relaxed));

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
