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

/*
 * The libseff library for shallow effects and handlers.
 * This is the only file that client code should include.
 */
#ifndef SEFF_H
#define SEFF_H

#include "seff_types.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
#define E extern "C"
#else
#define E
#endif

E bool seff_coroutine_init(seff_coroutine_t *, seff_start_fun_t *, void *);
E bool seff_coroutine_init_sized(seff_coroutine_t *, seff_start_fun_t *, void *, size_t);
E void seff_coroutine_release(seff_coroutine_t *);
E void seff_coroutine_reset(seff_coroutine_t *);

E seff_coroutine_t *seff_coroutine_new(seff_start_fun_t *, void *);
E seff_coroutine_t *seff_coroutine_new_sized(seff_start_fun_t *, void *, size_t);
E void seff_coroutine_delete(seff_coroutine_t *);

E __attribute__((no_split_stack)) void *seff_yield(seff_coroutine_t *self, void *arg);
E seff_coroutine_t *seff_locate_handler(effect_id effect);

E seff_coroutine_t *seff_current_coroutine(void);
// For debugging only. Do not use.
#ifdef STACK_POLICY_SEGMENTED
E __attribute__((no_split_stack)) void *seff_current_stack_top(void);
#endif

typedef void *(default_handler_t)(void *);
E default_handler_t *seff_set_default_handler(effect_id effect, default_handler_t *handler);
E default_handler_t *seff_get_default_handler(effect_id effect);

// Performance difference is massive between seff_perform being inlined or not
static inline void *seff_perform(effect_id eff_id, void *payload) {
    seff_coroutine_t *handler = seff_locate_handler(eff_id);
    if (handler) {
        seff_eff_t e;
        e.id = eff_id;
        e.resumption.coroutine = handler;
        e.resumption.sequence = e.resumption.coroutine->sequence;
        e.payload = payload;
        return seff_yield(handler, &e);
    } else {
        // Execute the handler in-place, since default handlers
        // are not allowed to pause the coroutine
        return seff_get_default_handler(eff_id)(payload);
    }
    // TODO: add an error message when there is no handler (rather than crashing)
    // This is a bit complicated since we don't want to have a call to fprintf directly
    // here, use a syscall wrapper
}

E __attribute__((noreturn)) void seff_throw(effect_id effect, void *payload);
E __attribute__((noreturn, no_split_stack)) void seff_return(seff_coroutine_t *k, void *result);

E seff_resumption_t seff_coroutine_start(seff_coroutine_t *coroutine);
E __attribute__((no_split_stack)) void *seff_handle(
    seff_resumption_t res, void *arg, effect_set handled);
E void *seff_resume(seff_resumption_t res, void *arg);

// TODO: this is architecture specific
#define MAKE_SYSCALL_WRAPPER(ret, fn, ...)                                                        \
    ret __attribute__((no_split_stack)) fn##_syscall_wrapper(__VA_ARGS__);                        \
    __asm__(#fn "_syscall_wrapper:"                                                               \
                "movq %rsp, %fs:_seff_paused_coroutine_stack@TPOFF;"                              \
                "movq %fs:_seff_system_stack@TPOFF, %rsp;"                                        \
                "movq %fs:0x70, %rax;" STACK_POLICY_SWITCH(                                       \
                    "movq %rax, %fs:_seff_paused_coroutine_stack_top@TPOFF;", "",                 \
                    "") "movq $0, %fs:0x70;"                                                      \
                        "callq " #fn ";"                                                          \
                        "movq %fs:_seff_paused_coroutine_stack@TPOFF, %rsp;" STACK_POLICY_SWITCH( \
                            "movq %fs:_seff_paused_coroutine_stack_top@TPOFF, %rcx;", "",         \
                            "") "movq %rcx, %fs:0x70;"                                            \
                                "retq;")

#define EFF_ID(name) __##name##_eff_id
#define EFF_PAYLOAD_T(name) __##name##_eff_payload
#define EFF_RET_T(name) __##name##_eff_ret
#define HANDLES(name) (1 << EFF_ID(name))

#define CASE_EFFECT(request, name, block)                                             \
    case __##name##_eff_id: {                                                         \
        __##name##_eff_payload payload = *(__##name##_eff_payload *)request->payload; \
        (void)payload;                                                                \
        block                                                                         \
    }

#define DEFINE_EFFECT(name, id, ret_val, payload) \
    typedef ret_val __##name##_eff_ret;           \
    static const int64_t __##name##_eff_id = id;  \
    typedef struct payload __##name##_eff_payload

#define PERFORM(name, ...)                                                           \
    ({                                                                               \
        __##name##_eff_payload __payload = {__VA_ARGS__};                            \
        (__##name##_eff_ret)(uintptr_t) seff_perform(__##name##_eff_id, &__payload); \
    })

#ifndef NDEBUG
#define PERFORM_DIRECT(coroutine, name, ...)                               \
    ({                                                                     \
        __##name##_eff_payload __payload = {__VA_ARGS__};                  \
        seff_eff_t __request = {__##name##_eff_id, &__payload};            \
        assert((coroutine->handled_effects & (HANDLES(name))));            \
        (__##name##_eff_ret)(uintptr_t) seff_yield(coroutine, &__request); \
    })
#else
#define PERFORM_DIRECT(coroutine, name, ...)                               \
    ({                                                                     \
        __##name##_eff_payload __payload = {__VA_ARGS__};                  \
        seff_eff_t __request = {__##name##_eff_id, &__payload};            \
        (__##name##_eff_ret)(uintptr_t) seff_yield(coroutine, &__request); \
    })
#endif

#define THROW(name, ...)                                  \
    ({                                                    \
        __##name##_eff_payload __payload = {__VA_ARGS__}; \
        seff_throw(__##name##_eff_id, &__payload);        \
    })

#endif
