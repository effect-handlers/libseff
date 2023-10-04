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
#include "mvar_stdio.h"

// Start the stdout_lock with a value
static mvar_t stdout_lock = MVAR_FILLED((void *)0x1);

MAKE_SYSCALL_WRAPPER(int, puts, const char *);

int mvar_puts(const char *msg) {
    void *lock = mvar_take(&stdout_lock);
    int ret = puts_syscall_wrapper(msg);
    mvar_put(&stdout_lock, lock);
    return ret;
}

MAKE_SYSCALL_WRAPPER(int, fputs, const char *, FILE *);

int mvar_fputs(const char *msg, FILE *f) {
    void *lock = mvar_take(&stdout_lock);
    int ret = fputs_syscall_wrapper(msg, f);
    mvar_put(&stdout_lock, lock);
    return ret;
}
