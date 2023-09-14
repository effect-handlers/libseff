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

#include "tl_queue.h"
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

void tl_queue_init(tl_queue_t *self, size_t log_size) {
    self->locked = false;
    self->head = 0;
    self->tail = 0;
    self->buffer = EMPTY;
    self->array = circular_array_new(log_size);
}

void tl_queue_acquire(tl_queue_t *self) {
    bool locked = false;
    while (!atomic_compare_exchange_weak_explicit(
        &self->locked, &locked, true, memory_order_acquire, memory_order_relaxed)) {
    }
}

void tl_queue_release(tl_queue_t *self) { RELAXED(store, &self->locked, false); }

void tl_queue_push(tl_queue_t *self, queue_elt_t elt) {
    tl_queue_acquire(self);
    circular_array_t *arr = self->array;
    int64_t head = self->head;
    int64_t tail = self->tail;

    if (tail - head > arr->size - 1) {
        circular_array_t *new_array = circular_array_resize(arr, tail, head);
        self->array = new_array;
        arr = new_array;
    }

    CA_SET(arr, tail, elt);
    self->tail = tail + 1;
    tl_queue_release(self);
    return;
}

void tl_queue_priority_push(tl_queue_t *self, queue_elt_t elt) {
    queue_elt_t old_buffer = self->buffer;
    self->buffer = elt;
    if (old_buffer) {
        tl_queue_push(self, old_buffer);
    }
}

queue_elt_t tl_queue_pop(tl_queue_t *self) {
    if (self->buffer != EMPTY) {
        queue_elt_t buffer = self->buffer;
        self->buffer = EMPTY;
        return buffer;
    }
    tl_queue_acquire(self);
    circular_array_t *arr = self->array;

    int64_t head = self->head;
    int64_t tail = self->tail;
    if (head == tail) {
        return EMPTY;
    }

    queue_elt_t elt = CA_GET(arr, head);
    self->head = head + 1;

    tl_queue_release(self);
    return elt;
}

queue_elt_t tl_queue_steal(tl_queue_t *self) {
    tl_queue_acquire(self);
    circular_array_t *arr = self->array;
    int64_t head = self->head;
    int64_t tail = self->tail;

    if (head == tail) {
        return EMPTY;
    }

    queue_elt_t elt = CA_GET(arr, head);
    self->head = head + 1;
    tl_queue_release(self);
    return elt;
}
