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

#ifndef SEFF_QUEUE_GENERIC_H
#define SEFF_QUEUE_GENERIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/poll.h>

#include "seff.h"

struct queue_t;

bool queue_init(struct queue_t *queue, size_t capacity);
size_t queue_size(struct queue_t *queue);
bool queue_empty(struct queue_t *queue);
bool queue_enqueue(struct queue_t *queue, task_t task);

task_t queue_dequeue(struct queue_t *queue);
void queue_destroy(struct queue_t *queue);

#endif
