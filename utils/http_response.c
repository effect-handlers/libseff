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

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "http_response.h"

const char *daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char *months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int build_response(char *buffer, size_t buff_size, char *header, char *content_type, void *body,
    int content_length) {
    size_t response_length = 0;
    time_t t = time(NULL);
    // DO NOT use localtime
    struct tm *tm = gmtime(&t);

    response_length +=
        snprintf(buffer + response_length, buff_size - response_length, "%s", header);
    response_length += snprintf(buffer + response_length, buff_size - response_length, "\r\n");
    // Date: <day-name>, <day> <month> <year> <hour>:<minute>:<second> GMT
    // This is considerable faster than strftime
    response_length += snprintf(buffer + response_length, buff_size - response_length,
        "Date: %s, %02d %s %04d %02d:%02d:%02d GMT\r\n", daysOfWeek[tm->tm_wday], tm->tm_mday,
        months[tm->tm_mon], tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
    // TODO, maybe allow closing?
    // response_length += snprintf(buffer + response_length, buff_size - response_length,
    // "Connection: close\r\n");
    response_length += snprintf(buffer + response_length, buff_size - response_length,
        "Content-Length: %d\r\n", content_length);
    response_length += snprintf(buffer + response_length, buff_size - response_length,
        "Content-Type: %s\r\n", content_type);
    response_length += snprintf(buffer + response_length, buff_size - response_length, "\r\n");
    if (content_length > 0) {
        response_length +=
            snprintf(buffer + response_length, buff_size - response_length, "%s", (char *)body);
    }

    if (response_length < 0 || response_length >= buff_size) {
        return -1;
    }

    return response_length;
}
