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
#ifndef SCHEFF_H
#define SCHEFF_H

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
#include "tl_queue.h"

#define QUEUE(t) tl_queue_##t

#define SCHEFF_DEBUG_COUNTERS \
    X(self_task_push)         \
    X(self_task_pop)          \
    X(self_task_abort)        \
    X(self_task_empty)        \
    X(self_task_poll_fail)    \
    X(stolen_task_ok)         \
    X(stolen_task_abort)      \
    X(stolen_task_empty)      \
    X(spinlock_fails)         \
    X(fork_requests)          \
    X(poll_requests)          \
    X(sleep_requests)         \
    X(wake_requests)          \
    X(return_requests)

struct task_t;
struct scheff_t;
struct worker_thread_t;
typedef struct scheff_t {
    size_t n_workers;
    _Atomic(int64_t) remaining_tasks;
#ifndef NDEBUG
    _Atomic(int64_t) task_counter;
    _Atomic(int64_t) max_tasks;
#endif
    struct worker_thread_t *workers;
} scheff_t;

bool scheff_init(scheff_t *self, size_t n_workers);
bool scheff_schedule(scheff_t *self, seff_start_fun_t fn, void *arg);
void scheff_run(scheff_t *scheduler);
void scheff_print_stats(scheff_t *scheduler);

void scheff_fork(seff_start_fun_t *fn, void *arg);

typedef bool(scheff_poll_condition_t)(void *);
void scheff_poll(scheff_poll_condition_t *poll, void *poll_arg);

struct scheff_waker_t;
typedef bool(scheff_wakeup_manager_t)(struct scheff_waker_t *waker, void *arg);
void scheff_sleep(scheff_wakeup_manager_t *must_sleep, void *arg);
void scheff_wake(struct scheff_waker_t *waker, bool resume);
void scheff_wake_all(size_t n_wakers, struct scheff_waker_t **waker, bool resume);

#endif // SCHEFF_H
