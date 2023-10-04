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
#ifndef MVAR_STDIO_H
#define MVAR_STDIO_H

#include <stdbool.h>
#include <stdio.h>

#include "seff.h"

int mvar_puts(const char *msg);

int mvar_fputs(const char *msg, FILE *f);

MAKE_SYSCALL_WRAPPER(int, sprintf, char *, const char *, ...);

#define mvar_printf(format, ...)                             \
    do {                                                     \
        char buf[100];                                       \
        sprintf_syscall_wrapper(buf, format, ##__VA_ARGS__); \
        mvar_fputs(buf, stdout);                             \
    } while (0);

#ifndef NDEBUG
#define deb_log(msg, ...) threadsafe_printf(msg, ##__VA_ARGS__)
#else
#define deb_log(msg, ...)
#endif

#endif // MVAR_STDIO_H
