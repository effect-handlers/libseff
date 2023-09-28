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

#ifndef SEFF_TK_QUEUE_H
#define SEFF_TK_QUEUE_H

#include <stddef.h>
#include <stdint.h>

struct task_t;
typedef struct task_t *queue_elt_t;

typedef struct {
    _Atomic(int64_t) head;
    _Atomic(int64_t) tail;
    queue_elt_t buffer;
    _Atomic(struct circular_array_t *) array;
} tk_queue_t;

void tk_queue_init(tk_queue_t *queue, size_t log_size);

#define EMPTY ((queue_elt_t)NULL)
#define ABORT ((queue_elt_t)(~(uintptr_t)NULL))

// Should only be called by the unique producer
void tk_queue_push(tk_queue_t *queue, queue_elt_t task);
// Should only be called by the unique producer
void tk_queue_priority_push(tk_queue_t *queue, queue_elt_t task);
// Should only be called by the unique producer
queue_elt_t tk_queue_pop(tk_queue_t *queue);
// Can be call by anyone
queue_elt_t tk_queue_steal(tk_queue_t *queue);

#endif
