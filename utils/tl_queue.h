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

#ifndef SEFF_TL_QUEUE_H
#define SEFF_TL_QUEUE_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct task_t;
typedef struct task_t *queue_elt_t;

typedef struct {
    _Atomic(bool) locked;
    int64_t head;
    int64_t tail;
    queue_elt_t buffer;
    struct circular_array_t *array;
} tl_queue_t;

void tl_queue_init(tl_queue_t *queue, size_t log_size);

#define EMPTY ((queue_elt_t)NULL)
#define ABORT ((queue_elt_t)(~(uintptr_t)NULL))

void tl_queue_push(tl_queue_t *queue, queue_elt_t task);
void tl_queue_priority_push(tl_queue_t *queue, queue_elt_t task);
queue_elt_t tl_queue_pop(tl_queue_t *queue);
queue_elt_t tl_queue_steal(tl_queue_t *queue);

#endif
