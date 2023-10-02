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
    int64_t start;
    int64_t size;
} queue_t;

bool queue_init(queue_t *queue, size_t capacity) {
    queue->tasks = calloc(capacity, sizeof(task_t));
    if (!queue->tasks)
        return false;
    queue->capacity = capacity;
    queue->start = 0;
    queue->size = 0;
    return true;
}
size_t queue_size(queue_t *queue) { return queue->size; }
bool queue_empty(queue_t *queue) { return queue->size == 0; }

bool queue_enqueue(queue_t *queue, task_t task) {
    assert(task.res.coroutine->state != FINISHED);
    if (queue->size == queue->capacity) {
        return false;
    }
    int64_t write_loc = (queue->start + queue->size) % queue->capacity;
    queue->tasks[write_loc] = task;
    queue->size += 1;
    return true;
}

task_t queue_dequeue(queue_t *queue) {
    if (queue->size <= 0) {
        return (task_t){{NULL, 0}, -1, 0};
    }
    int64_t read_loc = queue->start;
    queue->start = (queue->start + 1) % queue->capacity;
    queue->size -= 1;
    return queue->tasks[read_loc];
}
void queue_destroy(queue_t *queue) { free(queue->tasks); }
