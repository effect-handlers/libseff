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

#ifndef SEFF_ACTORS_H
#define SEFF_ACTORS_H

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "scheff.h"
#include "seff.h"

#define DEFAULT_MAILBOX_CAPACITY 128
struct actor_t;
typedef void *(actor_fn_t)(seff_coroutine_t *k, struct actor_t *self);
typedef struct actor_t {
    void **mailbox;
    _Atomic int64_t start;
    _Atomic int64_t write_loc;
    _Atomic int64_t reserved_size; // amount of locations
    _Atomic int64_t ready_size;    // amount of elements ready to be read
    size_t mailbox_capacity;
    actor_fn_t *behavior;
} actor_t;

bool actor_init(actor_t *actor, actor_fn_t *behavior, size_t mailbox_capacity);
void actor_destroy(actor_t *actor);
bool actor_insert_msg(actor_t *actor, void *msg);
void *actor_remove_msg(actor_t *actor);

void *actor_recv(actor_t *act);
void actor_send(actor_t *target, void *msg);
actor_t *fork_actor(actor_fn_t *behavior);

void actor_start(seff_start_fun_t main_actor, void *arg, size_t threads, bool print_sched_stats);

#endif
