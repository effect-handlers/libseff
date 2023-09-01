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

#include "actors.h"
#include "scheduler.h"

bool actor_init(actor_t *actor, actor_fn_t *behavior, size_t mailbox_capacity) {
    actor->mailbox = calloc(mailbox_capacity, sizeof(void *));
    if (!actor->mailbox)
        return false;
    atomic_store(&actor->write_loc, 0);
    atomic_store(&actor->reserved_size, 0);
    atomic_store(&actor->ready_size, 0);
    atomic_store(&actor->start, 0);
    actor->behavior = behavior;
    actor->mailbox_capacity = mailbox_capacity;
    return true;
}
void actor_destroy(actor_t *actor) {
    if (!actor->mailbox)
        return;
    free(actor->mailbox);
    actor->mailbox = NULL;
}
bool actor_insert_msg(actor_t *actor, void *msg) {
    assert(msg);

    int64_t old_size = atomic_load(&actor->reserved_size);
    if (old_size >= actor->mailbox_capacity) {
        return false;
    }

    if (!atomic_compare_exchange_strong(&actor->reserved_size, &old_size, old_size + 1)) {
        return false;
    }

    int64_t loc = atomic_fetch_add(&actor->write_loc, 1) % actor->mailbox_capacity;
    actor->mailbox[loc] = msg;
    atomic_fetch_add(&actor->ready_size, 1);
    return true;
}

bool actor_has_msg(actor_t *actor) { return atomic_load(&actor->ready_size) > 0; }
void *actor_remove_msg(actor_t *actor) {
    int64_t old_size = atomic_load(&actor->ready_size);
    if (old_size <= 0) {
        return false;
    }

    if (!atomic_compare_exchange_strong(&actor->ready_size, &old_size, old_size - 1)) {
        return false;
    }

    // start doesnt need to be atomic on single consumer
    int64_t read = atomic_fetch_add(&actor->start, 1) % actor->mailbox_capacity;

    void *result = actor->mailbox[read];
    actor->mailbox[read] = NULL;
    assert(result);
    atomic_fetch_sub(&actor->reserved_size, 1);
    return result;
}

void *actor_recv(actor_t *act) {
    void *msg = actor_remove_msg(act);
    while (!msg) {
        PERFORM(yield);
        msg = actor_remove_msg(act);
    }
    return msg;
}
void actor_send(actor_t *target, void *msg) {
    while (!actor_insert_msg(target, msg)) {
        // Wait until there is space in the target's mailbox
        PERFORM(yield);
    }
    return;
}

void *actor_thread(seff_coroutine_t *k, void *_arg) {
    actor_t *self = (actor_t *)_arg;

    return self->behavior(k, self);
}

actor_t *fork_actor(actor_fn_t *behavior) {
    actor_t *actor = malloc(sizeof(actor_t));
    actor_init(actor, behavior, DEFAULT_MAILBOX_CAPACITY);

    PERFORM(fork, actor_thread, actor);

    return actor;
}
