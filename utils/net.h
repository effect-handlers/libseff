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

#include <stdbool.h>
#include <stddef.h>

/**
 * Prepares a TCP socket to listen to connections.
 *
 * - ip: the ip to listen to
 * - port: the port, as a const char *
 * - non_blocking, reuse_address, and reuse_port: flags for different attributes
 * - listen_queue_size: the amount of connections to queue up waiting to be accepted
 */
int listen_tcp_socket(const char *ip, const char *port, bool non_blocking, bool reuse_address,
    bool reuse_port, int listen_queue_size);

/**
 * Async version of accept4, will try to return immediately, and await on the fd
 * if there's nothing to be accepted
 */
int await_accept4(int socket_fd);

/**
 * Async version of recv, will try to return immediately, and await on the fd
 * if there's nothing to be received
 */
int await_recv(int conn_fd, char *buffer, size_t bufsz);

/**
 * Async version of send, will try to return immediately, and await on the fd
 * if there's not enough buffer to send
 */
int await_send(int conn_fd, const char *buffer, size_t bufsz);

/**
 * Like await_send, but it will keep looping asynchronously until
 * all the data has been sent succesfuly
 */
int await_send_all(int conn_fd, const char *buffer, size_t bufsz);
