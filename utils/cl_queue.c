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

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>

#define RELAXED(op, ...) atomic_##op##_explicit(__VA_ARGS__, memory_order_relaxed)

const int64_t LOG_INITIAL_SIZE = 3;
const int64_t INITIAL_SIZE = 1 << LOG_INITIAL_SIZE;

typedef struct circular_array_t {
    int64_t size;
    int64_t mask;
    _Atomic(queue_elt_t) * buffer;
} circular_array_t;

circular_array_t *resize_array(circular_array_t *arr, int64_t bottom, int64_t top) {
    circular_array_t *new_array = malloc(sizeof(circular_array_t));

    int64_t new_size = arr->size << 1;
    int64_t new_mask = new_size - 1;
    _Atomic(queue_elt_t) *new_buffer = malloc(new_size * sizeof(queue_elt_t));

    new_array->size = new_size;
    new_array->mask = new_mask;
    new_array->buffer = new_buffer;

    _Atomic(queue_elt_t) *buffer = arr->buffer;
    int64_t old_mask = arr->mask;
    for (int64_t i = top; i < bottom; i++) {
        RELAXED(store, &new_buffer[i & new_mask], RELAXED(load, &buffer[i & old_mask]));
    }

    return new_array;
}

void cl_queue_init(queue_t *self) {
    self->bottom = 0;
    self->top = 0;
    self->array = malloc(sizeof(circular_array_t));
    self->array->size = INITIAL_SIZE;
    self->array->mask = INITIAL_SIZE - 1;
    self->array->buffer = malloc(INITIAL_SIZE * sizeof(queue_elt_t));
}

queue_elt_t queue_pop(queue_t *self) {
    int64_t bottom = RELAXED(load, &self->bottom) - 1;
    circular_array_t *arr = RELAXED(load, &self->array);
    RELAXED(store, &self->bottom, bottom);
    atomic_thread_fence(memory_order_seq_cst);

    int64_t top = RELAXED(load, &self->top);
    queue_elt_t elt;
    if (top <= bottom) {
        /* Non-empty queue */
        elt = RELAXED(load, &arr->buffer[bottom & arr->mask]);
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

void queue_push(queue_t *self, queue_elt_t elt) {
    int64_t bottom = RELAXED(load, &self->bottom);
    // Note this is *not* relaxed
    int64_t top = atomic_load_explicit(&self->top, memory_order_acquire);
    circular_array_t *arr = RELAXED(load, &self->array);
    if (bottom - top > arr->size - 1) {
        /* Queue is full! */
        circular_array_t *new_array = resize_array(arr, bottom, top);
        RELAXED(store, &self->array, new_array);
        /* TODO: we're leaking memory here */
        arr = new_array;
    }
    RELAXED(store, &arr->buffer[bottom & arr->mask], elt);
    // arr->buffer[bottom & arr->mask] = elt;
    atomic_thread_fence(memory_order_release);
    RELAXED(store, &self->bottom, bottom + 1);
}

queue_elt_t queue_steal(queue_t *self) {
    int64_t top = atomic_load_explicit(&self->top, memory_order_acquire);
    atomic_thread_fence(memory_order_seq_cst);
    int64_t bottom = atomic_load_explicit(&self->bottom, memory_order_acquire);
    queue_elt_t elt = EMPTY;
    if (top < bottom) {
        /* Non-empty queue */
        circular_array_t *arr = atomic_load_explicit(&self->array, memory_order_consume);
        elt = RELAXED(load, &arr->buffer[top & arr->mask]);
        if (!RELAXED(compare_exchange_strong, &self->top, &top, top + 1, memory_order_seq_cst)) {
            /* Failed the race */
            return ABORT;
        }
    }
    return elt;
}
