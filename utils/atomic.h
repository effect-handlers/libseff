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

#ifndef SEFF_ATOMIC_H
#define SEFF_ATOMIC_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RELAXED(op, ...) atomic_##op##_explicit(__VA_ARGS__, memory_order_relaxed)
#define ACQUIRE(op, ...) atomic_##op##_explicit(__VA_ARGS__, memory_order_acquire)
#define RELEASE(op, ...) atomic_##op##_explicit(__VA_ARGS__, memory_order_release)

#define ACQUIRE_LOCK(lock)                                           \
    bool was_locked = true;                                          \
    while (was_locked = ACQUIRE(exchange, lock, true), was_locked) { \
    }

#define RELEASE_LOCK(lock) RELEASE(store, lock, false);

#define SPINLOCK(lock, block)   \
    {                           \
        ACQUIRE_LOCK(lock)      \
                                \
        block                   \
                                \
            RELEASE_LOCK(lock); \
    }

#endif
