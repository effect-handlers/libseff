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

#include "cl_queue.h"
#include "atomic.h"
#include "circular_array.h"

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>

void cl_queue_init(cl_queue_t *self, size_t log_size) {
    self->bottom = 0;
    self->top = 0;
    self->array = circular_array_new(log_size);
}

queue_elt_t cl_queue_pop(cl_queue_t *self) {
    int64_t bottom = RELAXED(load, &self->bottom) - 1;
    circular_array_t *arr = RELAXED(load, &self->array);
    RELAXED(store, &self->bottom, bottom);
    atomic_thread_fence(memory_order_seq_cst);

    int64_t top = RELAXED(load, &self->top);
    queue_elt_t elt;
    if (top <= bottom) {
        /* Non-empty queue */
        elt = CA_GET(arr, bottom);
        if (top == bottom) {
            /* We just raced for the last element */
            if (!RELAXED(
                    compare_exchange_strong, &self->top, &top, top + 1, memory_order_seq_cst)) {
                /* Lost the race */
                elt = EMPTY;
            }
            RELAXED(store, &self->bottom, bottom + 1);
        }
    } else {
        elt = EMPTY;
        RELAXED(store, &self->bottom, bottom + 1);
    }
    return elt;
}

void cl_queue_push(cl_queue_t *self, queue_elt_t elt) {
    int64_t bottom = RELAXED(load, &self->bottom);
    // Note this is *not* relaxed
    int64_t top = atomic_load_explicit(&self->top, memory_order_acquire);
    circular_array_t *arr = RELAXED(load, &self->array);
    if (bottom - top > arr->size - 1) {
        /* Queue is full! */
        circular_array_t *new_array = circular_array_resize(arr, bottom, top);
        RELAXED(store, &self->array, new_array);
        /* TODO: we're leaking memory here */
        arr = new_array;
    }
    CA_SET(arr, bottom, elt);
    atomic_thread_fence(memory_order_release);
    RELAXED(store, &self->bottom, bottom + 1);
}

void cl_queue_priority_push(cl_queue_t *self, queue_elt_t elt) { cl_queue_push(self, elt); }

queue_elt_t cl_queue_steal(cl_queue_t *self) {
    int64_t top = atomic_load_explicit(&self->top, memory_order_acquire);
    atomic_thread_fence(memory_order_seq_cst);
    int64_t bottom = atomic_load_explicit(&self->bottom, memory_order_acquire);
    queue_elt_t elt = EMPTY;
    if (top < bottom) {
        /* Non-empty queue */
        circular_array_t *arr = atomic_load_explicit(&self->array, memory_order_consume);
        elt = CA_GET(arr, top);
        if (!RELAXED(compare_exchange_strong, &self->top, &top, top + 1, memory_order_seq_cst)) {
            /* Failed the race */
            return ABORT;
        }
    }
    return elt;
}
