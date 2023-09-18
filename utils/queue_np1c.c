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
#include <string.h>

typedef struct {
    task_t task;
    _Atomic bool valid;
} queue_node_t;

typedef struct queue_t {
    queue_node_t *tasks;
    size_t capacity;
    int64_t read_loc;
    _Atomic int64_t size;
    _Atomic int64_t occupied_a;
    _Atomic int64_t write_loc_a;
} queue_t;

// We use it to compare against non filled tasks
task_t zeroed_task;
bool queue_init(queue_t *queue, size_t capacity) {
#ifndef NDEBUG
    memset(&zeroed_task, 0, sizeof(task_t));
#endif
    queue->tasks = calloc(capacity, sizeof(queue_node_t));
    if (!queue->tasks)
        return false;
    for (int i = 0; i < capacity; i++) {
#ifndef NDEBUG
        memset(&queue->tasks[i].task, 0, sizeof(task_t));
#endif
        atomic_store(&queue->tasks[i].valid, false);
    }
    queue->capacity = capacity;
    queue->read_loc = 0;
    atomic_store(&queue->write_loc_a, 0);
    atomic_store(&queue->occupied_a, 0);
    atomic_store(&queue->size, 0);
    return true;
}
size_t queue_size(queue_t *queue) {
    return atomic_load_explicit(&queue->size, memory_order_relaxed);
}
bool queue_empty(queue_t *queue) { return queue_size(queue) == 0; }

_Atomic int n_enqueue;
_Atomic int n_enqueue_fail;

int n_enqueues(void) { return atomic_load(&n_enqueue); }
int n_failed_enqueues(void) { return atomic_load(&n_enqueue_fail); }

bool queue_enqueue(queue_t *queue, task_t task) {
#ifndef NDEBUG
    atomic_fetch_add_explicit(&n_enqueue, 1, memory_order_relaxed);
#endif
    int64_t occupied = atomic_fetch_add_explicit(&queue->occupied_a, 1, memory_order_seq_cst);
    if (occupied > 0 && occupied >= queue->capacity) {
        atomic_fetch_sub_explicit(&queue->occupied_a, 1, memory_order_seq_cst);
#ifndef NDEBUG
        atomic_fetch_add_explicit(&n_enqueue_fail, 1, memory_order_relaxed);
#endif
        return false;
    }
    int64_t write_loc_val = atomic_fetch_add_explicit(&queue->write_loc_a, 1, memory_order_relaxed);
    int64_t write_loc = write_loc_val % queue->capacity;
#ifndef NDEBUG
    assert(memcmp(&queue->tasks[write_loc].task, &zeroed_task, sizeof(task_t)) == 0);
    assert(memcmp(&task, &zeroed_task, sizeof(task_t)) != 0);
#endif
    queue->tasks[write_loc].task = task;
    bool flag =
        atomic_fetch_or_explicit(&queue->tasks[write_loc].valid, true, memory_order_seq_cst);
    (void)flag;
#ifndef NDEBUG
    assert(!flag);
#endif
    atomic_fetch_add_explicit(&queue->size, 1, memory_order_relaxed);
    return true;
}

task_t queue_dequeue(queue_t *queue) {
    bool flag = atomic_fetch_and(&queue->tasks[queue->read_loc].valid, false);
    if (!flag) {
        return (task_t){0};
    }

    task_t task = queue->tasks[queue->read_loc].task;
#ifndef NDEBUG
    assert(memcmp(&task, &zeroed_task, sizeof(task_t)) != 0);
    memset(&queue->tasks[queue->read_loc].task, 0, sizeof(task_t));
#endif
    atomic_fetch_sub_explicit(&queue->occupied_a, 1, memory_order_seq_cst);
    queue->read_loc = (queue->read_loc + 1) % queue->capacity;
    atomic_fetch_sub_explicit(&queue->size, 1, memory_order_relaxed);
    return task;
}

void queue_destroy(queue_t *queue) { free(queue->tasks); }
