#pragma once

#include "seff_types.h"
#include <assert.h>
#include <stdbool.h>
#include <xmmintrin.h>

typedef struct task_t {
    seff_coroutine_t *coro;
    void *extra;
} task_t;

typedef struct coro_queue {
    task_t *coros;
    int64_t start;
    int64_t size;
} coro_queue;

typedef struct search_args {
    int const *first;
    size_t len;
    int key;
} search_args;

// Prefetch
void prefetch_c(const char *x) { _mm_prefetch(x, _MM_HINT_NTA); }

const size_t capacity = 256;

bool coro_queue_init(coro_queue *queue) {
    queue->coros = calloc(capacity, sizeof(task_t));
    if (!queue->coros)
        return false;
    queue->start = 0;
    queue->size = 0;
    return true;
}

size_t coro_queue_size(coro_queue *queue) { return queue->size; }
bool coro_queue_empty(coro_queue *queue) { return queue->size == 0; }

bool coro_queue_enqueue(coro_queue *queue, task_t task) {
    if (queue->size == capacity) {
        return false;
    }
    int64_t write_loc = (queue->start + queue->size) % capacity;
    queue->coros[write_loc] = task;
    queue->size += 1;
    return true;
}

task_t coro_queue_dequeue(coro_queue *queue) {
    if (queue->size <= 0) {
        return (task_t){NULL, NULL};
    }
    task_t ret = queue->coros[queue->start];
    queue->start = (queue->start + 1) % capacity;
    queue->size -= 1;
    return ret;
}
void coro_queue_destroy(coro_queue *queue) { free(queue->coros); }
