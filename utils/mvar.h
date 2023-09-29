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

#ifndef MVAR_H
#define MVAR_H

#include "scheff.h"

typedef struct queue_node {
    struct scheff_waker_t *waker;
    // If this node is waiting for an object, set it in this value and then wake up
    // if it's waiting for someone to empty it, take it from here, when it's awaken the pointed
    // pointer may be overriden
    void **value;
    struct queue_node *next;
} queue_node;

typedef struct __attribute__((__packed__)) value_queue {
    void *value;
    queue_node *queue;
} value_queue;

typedef struct mvar_t {
    _Atomic value_queue vq;
} mvar_t;

void mvar_init(mvar_t *);

void mvar_put(mvar_t *, void *);

void *mvar_take(mvar_t *);

#endif // MVAR_H
