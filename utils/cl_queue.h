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

#ifndef SEFF_CL_QUEUE_H
#define SEFF_CL_QUEUE_H

#include <stddef.h>
#include <stdint.h>

struct task_t;

typedef struct queue_t {
    _Atomic(int64_t) top;
    _Atomic(int64_t) bottom;
    _Atomic(struct circular_array_t *) array;
} queue_t;

typedef struct task_t *queue_elt_t;

void cl_queue_init(queue_t *queue);

#define EMPTY ((queue_elt_t)NULL)
#define ABORT ((queue_elt_t)(~(uintptr_t)NULL))

void queue_push(queue_t *queue, queue_elt_t task);
queue_elt_t queue_pop(queue_t *queue);
queue_elt_t queue_steal(queue_t *queue);

#endif
