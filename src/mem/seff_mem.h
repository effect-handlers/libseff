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

#ifndef SEFF_MEM_H
#define SEFF_MEM_H

#include "seff_types.h"
#include <stdlib.h>

#ifdef __cplusplus
#define E extern "C"
#else
#define E
#endif

// TODO: this module should also be responsible for deleting stack frames
// Given a frame size and a pointer to the rsp, return the new stack frame, and modify rsp so that
// it points to the top of the stack
E __attribute__((no_split_stack)) seff_frame_ptr_t init_stack_frame(size_t frame_size, char **rsp);

#endif
