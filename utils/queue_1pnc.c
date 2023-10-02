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

#include "queue_generic.h"

#include <assert.h>
#include <stdatomic.h>

typedef struct queue_t {
    task_t *tasks;
    size_t capacity;
    _Atomic int64_t read_loc_a;
    _Atomic int64_t size_a;
    int64_t write_loc;
} queue_t;

bool queue_init(queue_t *queue, size_t capacity) {
    queue->tasks = calloc(capacity, sizeof(task_t));
    if (!queue->tasks)
        return false;
    queue->capacity = capacity;
    atomic_store(&queue->read_loc_a, 0);
    atomic_store(&queue->size_a, 0);
    queue->write_loc = 0;
    return true;
}
size_t queue_size(queue_t *queue) { return atomic_load(&queue->size_a); }
bool queue_empty(queue_t *queue) { return queue_size(queue) == 0; }
bool queue_enqueue(queue_t *queue, task_t task) {
    assert(task.res.coroutine->state != FINISHED);
    if (atomic_load_explicit(&queue->size_a, memory_order_relaxed) == queue->capacity) {
        return false;
    }
    queue->tasks[queue->write_loc] = task;
    queue->write_loc = (queue->write_loc + 1) % queue->capacity;
    atomic_fetch_add_explicit(&queue->size_a, 1, memory_order_relaxed);
    return true;
}

_Atomic int n_dequeue;
_Atomic int n_dequeue_fail;
int n_dequeues(void) { return atomic_load(&n_dequeue); }
int n_failed_dequeues(void) { return atomic_load(&n_dequeue_fail); }

task_t queue_dequeue(queue_t *queue) {
#ifndef NDEBUG
    atomic_fetch_add_explicit(&n_dequeue, 1, memory_order_relaxed);
#endif
    int64_t remaining = atomic_fetch_sub_explicit(&queue->size_a, 1, memory_order_relaxed);
    if (remaining <= 0) {
        atomic_fetch_add_explicit(&queue->size_a, 1, memory_order_relaxed);
#ifndef NDEBUG
        atomic_fetch_add_explicit(&n_dequeue_fail, 1, memory_order_relaxed);
#endif
        return (task_t){{NULL, 0}, -1, 0};
    }
    int64_t read_loc =
        atomic_fetch_add_explicit(&queue->read_loc_a, 1, memory_order_relaxed) % queue->capacity;
    assert(queue->tasks[read_loc].res.coroutine->state != FINISHED);
    return queue->tasks[read_loc];
}
void queue_destroy(queue_t *queue) { free(queue->tasks); }
