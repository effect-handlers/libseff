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

#ifndef SEFF_SCHEDULER_H
#define SEFF_SCHEDULER_H

#include <assert.h>
#include <poll.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/poll.h>

#include "seff.h"

#define GUARD(mutex, block)                \
    {                                      \
        pthread_mutex_lock(mutex);         \
        block pthread_mutex_unlock(mutex); \
    }
static pthread_mutex_t puts_mutex;

#define threadsafe_puts(str) GUARD(&puts_mutex, { puts(str); });
#define threadsafe_printf(...) GUARD(&puts_mutex, { printf(__VA_ARGS__); })

DEFINE_EFFECT(yield, 0, void, {});
DEFINE_EFFECT(fork, 1, void, {
    seff_start_fun_t *function;
    void *argument;
});

/* Note that all of these are fully identical to the corresponding both POLL and EPOLL events */
typedef enum : uint32_t {
    READ = POLLIN,
    WRITE = POLLOUT,
    HANGUP = POLLHUP,
    ERROR = POLLERR,
    ET = EPOLLET
} event_t;

DEFINE_EFFECT(await, 2, event_t, {
    int fd;
    event_t events;
});

#endif
