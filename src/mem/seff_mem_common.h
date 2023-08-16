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

#ifndef SEFF_MEM_COMMON_H
#define SEFF_MEM_COMMON_H

extern __thread seff_coroutine_t *_seff_current_coroutine;
extern __thread void *_seff_system_stack;

size_t min_segment_size;
size_t default_frame_size;
size_t syscall_segment_size;

size_t read_env_size(const char *env_var, size_t default_val) {
    const char *env_val = getenv(env_var);

    if (!env_val) {
        return default_val;
    }

    size_t result;
    if (!sscanf(env_val, "%zu", &result)) {
#ifndef NDEBUG
        printf("Invalid value %s for environment variable %s\n", env_val, env_var);
        exit(-1);
#else
        return default_val;
#endif
    }
    return result;
}

__attribute__((visibility("hidden"), constructor(65535))) void seff_mem_global_init(void);
void seff_mem_global_init(void) {
    // Here go all the calls to anything that might get invoked by memory management code
    malloc(0);
    free(NULL);
    int src, dst;
    memcpy(&dst, &src, 0);

#ifdef STACK_POLICY_SEGMENTED
    min_segment_size = read_env_size("LIBSEFF_MIN_SEGMENT_SIZE", DEFAULT_MIN_SEGMENT_SIZE);
    syscall_segment_size =
        read_env_size("LIBSEFF_SYSCALL_SEGMENT_SIZE", DEFAULT_SYSCALL_SEGMENT_SIZE);
#endif
    default_frame_size = read_env_size("LIBSEFF_DEFAULT_FRAME_SIZE", DEFAULT_DEFAULT_FRAME_SIZE);

#ifndef NDEBUG
    printf("libseff using %s\nMinimum segment size: %lu\nSyscall segment size: "
           "%lu\nInitial "
           "frame size: %lu\n\n",
        STACK_POLICY_SWITCH("segmented stacks", "fixed-size stacks", "virtual memory stacks"),
        min_segment_size, syscall_segment_size, default_frame_size);
#endif
}

#endif
