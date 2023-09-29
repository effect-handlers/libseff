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

#include <stdatomic.h>

#include "mvar.h"

void mvar_init(mvar_t *mv) {
    value_queue vq;
    vq.value = NULL;
    vq.queue = NULL;
    atomic_store(&mv->vq, vq);
}

typedef struct {
    bool enqueued;
    value_queue vq;
    _Atomic value_queue *original_vq;
    void **value;
} put_full_args;

bool put_full_must_sleep(struct scheff_waker_t *waker, void *_arg) {
    put_full_args *arg = (put_full_args *)_arg;
    arg->enqueued = true;
    value_queue new_vq;
    new_vq.value = arg->vq.value;
    new_vq.queue = malloc(sizeof(queue_node));
    if (!new_vq.queue) {
        arg->enqueued = false;
    } else {
        new_vq.queue->next = arg->vq.queue;
        new_vq.queue->value = arg->value;
        new_vq.queue->waker = waker;
        if (!atomic_compare_exchange_strong(arg->original_vq, &arg->vq, new_vq)) {
            // Too late, try again
            arg->enqueued = false;
        }
    }

    return arg->enqueued;
}

void mvar_put(mvar_t *mv, void *value) {
    while (true) {
        value_queue vq = atomic_load(&mv->vq);
        if (!vq.value) {
            // It's empty
            value_queue new_vq;
            if (!vq.queue) {
                // There are no waiters
                new_vq.value = value;
                new_vq.queue = NULL;
                if (!atomic_compare_exchange_strong(&mv->vq, &vq, new_vq)) {
                    // Too late, try again
                    continue;
                }
            } else {
                // Wake up the first waker
                queue_node *top = vq.queue;
                new_vq.value = NULL;
                new_vq.queue = top->next;
                if (!atomic_compare_exchange_strong(&mv->vq, &vq, new_vq)) {
                    // Too late, try again
                    continue;
                }
                // Got it, wake it up
                *(top->value) = value;
                scheff_wake(top->waker, true);
                free(top); // syscall wrtap it
            }

        } else {
            // There's already some value, add me to the top
            // This is actually a stack, we should change it into a queue
            put_full_args args;
            args.original_vq = &mv->vq;
            args.vq = vq;
            args.value = &value;
            scheff_sleep(put_full_must_sleep, &args);
            if (!args.enqueued) {
                // Too late, try again
                continue;
            }
        }
        break;
    }
}

typedef struct {
    bool enqueued;
    value_queue vq;
    _Atomic value_queue *original_vq;
    void **value;
} take_empty_args;

bool take_empty_must_sleep(struct scheff_waker_t *waker, void *_arg) {
    take_empty_args *arg = (take_empty_args *)_arg;
    arg->enqueued = true;
    value_queue new_vq;
    new_vq.value = arg->vq.value;
    new_vq.queue = malloc(sizeof(queue_node));
    if (!new_vq.queue) {
        arg->enqueued = false;
    } else {
        new_vq.queue->next = arg->vq.queue;
        new_vq.queue->value = arg->value;
        new_vq.queue->waker = waker;
        if (!atomic_compare_exchange_strong(arg->original_vq, &arg->vq, new_vq)) {
            // Too late, try again
            arg->enqueued = false;
        }
    }

    return arg->enqueued;
}

void *mvar_take(mvar_t *mv) {
    while (true) {
        value_queue vq = atomic_load(&mv->vq);
        if (vq.value) {
            // It's not empty
            value_queue new_vq;
            if (!vq.queue) {
                // There are no waiters
                new_vq.value = NULL;
                new_vq.queue = NULL;
                if (!atomic_compare_exchange_strong(&mv->vq, &vq, new_vq)) {
                    // Too late, try again
                    continue;
                }
                return vq.value;
            } else {
                // Wake up the first waker
                queue_node *top = vq.queue;
                new_vq.value = *top->value;
                new_vq.queue = top->next;
                void *value = vq.value;
                if (!atomic_compare_exchange_strong(&mv->vq, &vq, new_vq)) {
                    // Too late, try again
                    continue;
                }
                // Got it, wake it up
                scheff_wake(top->waker, true);
                free(top); // syscall wrtap it
                return value;
            }

        } else {
            // It's empty, add me to the top of the wakers
            // This is actually a stack, we should change it into a queue
            take_empty_args args;
            args.original_vq = &mv->vq;
            args.vq = vq;
            void *value;
            args.value = &value;
            scheff_sleep(take_empty_must_sleep, &args);
            if (!args.enqueued) {
                // Too late, try again
                continue;
            }
            // At this point, take_put should've inserted something on our value
            // It's a leap of faith
            return value;
        }
        break;
    }
}
