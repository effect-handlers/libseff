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
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cl_queue.h"
#include "seff.h"
#include "tk_queue.h"

#define QUEUE(t) tk_queue_##t

#undef NDEBUG

struct task_t;
struct future_t;
struct scheff_t;
struct worker_thread_t;

typedef struct task_list_t {
    struct task_t *task;
    struct task_list_t *rest;
} task_list_t;

typedef enum { WAITING, WRITING, READY, CANCELLED } future_state_t;
typedef struct future_t {
    _Atomic(bool) blocked;
    _Atomic(future_state_t) state;
    void *result;
    struct task_t *waiter;
    task_list_t *waiters;
} future_t;

inline future_t new_future(void) { return (future_t){false, WAITING, NULL, NULL, NULL}; }

typedef struct {
    bool wake;
    void *result;
} wakeup_t;
typedef wakeup_t(scheff_wakeup_fn_t)(void *);

typedef struct task_t {
    seff_coroutine_t coroutine;
    scheff_wakeup_fn_t *wakeup_fn;
    void *wakeup_arg;
} task_t;

#define SCHEFF_DEBUG_COUNTERS                                                                      \
    X(self_task_push)                                                                              \
    X(self_task_pop)                                                                               \
    X(self_task_abort)                                                                             \
    X(self_task_empty)                                                                             \
    X(self_task_asleep)                                                                            \
    X(stolen_task_ok) X(stolen_task_abort) X(stolen_task_empty) X(spinlock_fails) X(fork_requests) \
        X(await_requests) X(notify_requests) X(suspend_requests) X(return_requests)

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

typedef struct scheff_t {
    size_t n_workers;
    _Atomic(int64_t) remaining_tasks;
#ifndef NDEBUG
    _Atomic(int64_t) max_tasks;
#endif
    worker_thread_t *workers;
} scheff_t;

bool scheff_init(scheff_t *self, size_t n_workers);
void scheff_schedule(scheff_t *self, seff_start_fun_t fn, void *arg);
void scheff_run(scheff_t *scheduler);
void scheff_print_stats(scheff_t *scheduler);

void scheff_fork(seff_start_fun_t *fn, void *arg);
void *scheff_await(future_t *fut);
bool scheff_fulfill(future_t *fut, void *result, bool resume);

void *scheff_suspend(scheff_wakeup_fn_t *wk, void *wk_arg);

void scheff_async(seff_start_fun_t *fn, void *arg, future_t *fut);
