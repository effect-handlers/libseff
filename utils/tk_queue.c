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

#include "tk_queue.h"
#include "circular_array.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define RELAXED(op, ...) atomic_##op##_explicit(__VA_ARGS__, memory_order_relaxed)
#define ACQUIRE(op, ...) atomic_##op##_explicit(__VA_ARGS__, memory_order_acquire)
#define RELEASE(op, ...) atomic_##op##_explicit(__VA_ARGS__, memory_order_release)

void tk_queue_init(tk_queue_t *self, size_t log_size) {
    self->bottom = 0;
    self->top = 0;
    self->head = EMPTY;
    self->array = circular_array_new(log_size);
}

void tk_queue_push(tk_queue_t *self, queue_elt_t elt) {
    circular_array_t *arr = self->array;
    while (true) {
        int64_t top = ACQUIRE(load, &self->top);
        int64_t bottom = self->bottom;

        if (bottom - top > arr->size - 1) {
        } else {
            arr->buffer[bottom & arr->mask] = elt;
            RELEASE(store, &self->bottom, bottom + 1);
            return;
        }
    }
}

void tk_queue_priority_push(tk_queue_t *self, queue_elt_t elt) {
    queue_elt_t old_head = self->head;
    self->head = elt;
    if (old_head) {
        tk_queue_push(self, old_head);
    }
}

queue_elt_t tk_queue_pop(tk_queue_t *self) {
    if (self->head != EMPTY) {
        queue_elt_t head = self->head;
        self->head = EMPTY;
        return head;
    }
    circular_array_t *arr = self->array;
    while (true) {
        int64_t top = ACQUIRE(load, &self->top);
        int64_t bottom = self->bottom;

        if (top == bottom) {
            return EMPTY;
        }

        queue_elt_t elt = arr->buffer[top & arr->mask];

        if (atomic_compare_exchange_strong_explicit(
                &self->top, &top, top + 1, memory_order_release, memory_order_relaxed)) {
            return elt;
        }
    }
}

queue_elt_t tk_queue_steal(tk_queue_t *self) {
    circular_array_t *arr = self->array;
    while (true) {
        int64_t top = ACQUIRE(load, &self->top);
        int64_t bottom = ACQUIRE(load, &self->bottom);

        if (top == bottom) {
            return EMPTY;
        }

        queue_elt_t elt = arr->buffer[top & arr->mask];

        if (atomic_compare_exchange_strong_explicit(
                &self->top, &top, top + 1, memory_order_release, memory_order_relaxed)) {
            return elt;
        }
    }
}
