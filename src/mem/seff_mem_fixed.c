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
#define DEFAULT_DEFAULT_FRAME_SIZE 8 * 1024
#include "seff_mem_common.h"

seff_frame_ptr_t init_stack_frame(size_t frame_size, char **rsp) {
    seff_frame_ptr_t frame = malloc(frame_size);

    if (!frame) {
        exit(-1);
    }

#ifndef NDEBUG
    char *ptr = (char *)frame;
    for (int i = 0; i < frame_size; i++) {
        ptr[i] = 0x13;
    }
#endif

    *rsp = (char *)frame + frame_size;
    return frame;
}
