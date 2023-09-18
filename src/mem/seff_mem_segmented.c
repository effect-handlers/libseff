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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "seff_mem.h"

// These numbers change between policies
#define DEFAULT_MIN_SEGMENT_SIZE 0
#define DEFAULT_SYSCALL_SEGMENT_SIZE 8 * 1024
#define DEFAULT_DEFAULT_FRAME_SIZE 1024
#include "seff_mem_common.h"

size_t current_stack_top(void);
__asm__("current_stack_top:"
        "movq %fs:0x70,%rax;"
        "ret;");

seff_stack_segment_t *init_segment(size_t frame_size) {
    size_t overhead = sizeof(seff_stack_segment_t);
    seff_stack_segment_t *segment = malloc(frame_size + overhead);

    if (!segment) {
        exit(-1);
    }

#ifndef NDEBUG
    char *ptr = (char *)segment;
    for (int i = 0; i < frame_size + overhead; i++) {
        ptr[i] = 0x13;
    }
#endif
    segment->prev = NULL;
    segment->next = NULL;
    segment->size = frame_size;

    return segment;
}

seff_frame_ptr_t init_stack_frame(size_t frame_size) { return init_segment(frame_size); }

#define ATTRS __attribute__((no_split_stack, visibility("hidden"), flatten))

/* Internal seff_mem functions called by the runtime */
ATTRS void *seff_mem_allocate_frame(size_t *frame_size, void *old_stack, size_t param_size);
ATTRS void *seff_mem_release_frame(void);

#define LINK(first, second)   \
    {                         \
        first->next = second; \
        second->prev = first; \
    }
void *seff_mem_allocate_frame(size_t *pframe_size, void *old_stack, size_t param_size) {
    size_t frame_size = *pframe_size;
    seff_stack_segment_t *current = _seff_current_coroutine->frame_ptr;
    assert(current != NULL);

    size_t new_size = frame_size + param_size;
    if (new_size < min_segment_size)
        new_size = min_segment_size;

    seff_stack_segment_t *new_segment;
    // FIXME: also account for coroutine pause overhead?
    if (current->next == NULL) {
        new_segment = init_segment(new_size);
        LINK(current, new_segment);
    } else if (current->next->size < frame_size + param_size) {
        // TODO: Here we could delete the smaller segment
        new_segment = init_segment(new_size);
        LINK(new_segment, current->next);
        LINK(current, new_segment);
    } else {
        new_segment = current->next;
    }

    _seff_current_coroutine->frame_ptr = new_segment;
    *pframe_size = new_segment->size - param_size;

    /*
     * Align the returned stack to a 16-byte boundary.
     * FIXME: in theory this could cause problems because it means
     * the stack frame will have LESS available space than advertised
     * FIXME: do this properly instead of this garbage
     */
    char *new_stack = (char *)(new_segment + 1) + new_segment->size - param_size;
    while (((uintptr_t)new_stack) % 16 != 0) {
        new_stack -= 1;
    }
    memcpy(new_stack, old_stack, param_size);
    assert(((uintptr_t)new_stack) % 16 == 0);

    return new_stack;
}

void *seff_mem_release_frame(void) {
    seff_stack_segment_t *current = _seff_current_coroutine->frame_ptr;
    _seff_current_coroutine->frame_ptr = current->prev;
    void *stack_top = (char *)current->prev + sizeof(seff_stack_segment_t);
    return stack_top;
}

typedef struct {
    void *(*start_routine)(void *);
    void *arg;
} seff_mem_start_routine_args;

extern void seff_mem_thread_init(void);
void *seff_mem_start_routine(void *arg) {
    seff_mem_start_routine_args args = *(seff_mem_start_routine_args *)arg;
    free(arg);
    seff_mem_thread_init();
    return args.start_routine(args.arg);
}

extern __attribute__((weak)) int __real_pthread_create(
    pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int __wrap_pthread_create(
    pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    seff_mem_start_routine_args *args = malloc(sizeof(seff_mem_start_routine_args));
    args->start_routine = start_routine;
    args->arg = arg;
    return __real_pthread_create(thread, attr, seff_mem_start_routine, args);
}
