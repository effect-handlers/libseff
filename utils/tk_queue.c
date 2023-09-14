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
#include "atomic.h"
#include "circular_array.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void tk_queue_init(tk_queue_t *self, size_t log_size) {
    self->head = 0;
    self->tail = 0;
    self->buffer = EMPTY;
    self->array = circular_array_new(log_size);
}

void tk_queue_push(tk_queue_t *self, queue_elt_t elt) {
    circular_array_t *arr = RELAXED(load, &self->array);
    int64_t head = ACQUIRE(load, &self->head);
    int64_t tail = self->tail;

    if (tail - head > arr->size - 1) {
        circular_array_t *new_array = circular_array_resize(arr, tail, head);
        RELAXED(store, &self->array, new_array);
        arr = new_array;
    }

    CA_SET(arr, tail, elt);
    RELEASE(store, &self->tail, tail + 1);
    return;
}

void tk_queue_priority_push(tk_queue_t *self, queue_elt_t elt) {
    queue_elt_t old_buffer = self->buffer;
    self->buffer = elt;
    if (old_buffer) {
        tk_queue_push(self, old_buffer);
    }
}

queue_elt_t tk_queue_pop(tk_queue_t *self) {
    if (self->buffer != EMPTY) {
        queue_elt_t buffer = self->buffer;
        self->buffer = EMPTY;
        return buffer;
    }
    circular_array_t *arr = self->array;
    while (true) {
        int64_t head = ACQUIRE(load, &self->head);
        int64_t tail = self->tail;

        if (head == tail) {
            return EMPTY;
        }

        queue_elt_t elt = CA_GET(arr, head);

        if (atomic_compare_exchange_strong_explicit(
                &self->head, &head, head + 1, memory_order_release, memory_order_relaxed)) {
            return elt;
        }
    }
}

queue_elt_t tk_queue_steal(tk_queue_t *self) {
    circular_array_t *arr = self->array;
    while (true) {
        int64_t head = ACQUIRE(load, &self->head);
        int64_t tail = RELAXED(load, &self->tail);

        if (head == tail) {
            return EMPTY;
        }

        queue_elt_t elt = CA_GET(arr, head);

        if (atomic_compare_exchange_strong_explicit(
                &self->head, &head, head + 1, memory_order_release, memory_order_relaxed)) {
            return elt;
        }
    }
}
