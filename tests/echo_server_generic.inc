#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "seff.h"

int set_nonblock(int iSock) {
    int iFlags;

    iFlags = fcntl(iSock, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    int ret = fcntl(iSock, F_SETFL, iFlags);
    return ret;
}

void set_addr(const char *pszIP, const unsigned short shPort, struct sockaddr_in *addr) {
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(shPort);
    int nIP = 0;
    if (!pszIP || '\0' == *pszIP || 0 == strcmp(pszIP, "0") || 0 == strcmp(pszIP, "0.0.0.0") ||
        0 == strcmp(pszIP, "*")) {
        nIP = htonl(INADDR_ANY);
    } else {
        nIP = inet_addr(pszIP);
    }
    addr->sin_addr.s_addr = nIP;
}

int open_tcp_socket(const unsigned short shPort, const char *pszIP, bool bReuse) {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd >= 0) {
        if (shPort != 0) {
            if (bReuse) {
                int nReuseAddr = 1;
                setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &nReuseAddr, sizeof(nReuseAddr));
            }
            struct sockaddr_in addr;
            set_addr(pszIP, shPort, &addr);
            int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
            if (ret != 0) {
                close(fd);
                return -1;
            }
        }
    }
    return fd;
}

MAKE_SYSCALL_WRAPPER(int, accept, int, void*, void*);
int await_connection(int socket_fd) {
    event_t revents = PERFORM(await, socket_fd, READ);
    if (HANGUP & revents)
        return 0;
    if (!(READ & revents))
        return -1;

    return accept_syscall_wrapper(socket_fd, NULL, NULL);
}

MAKE_SYSCALL_WRAPPER(int, recv, int, void*, size_t, int);

#define BUF_SIZE 256
int await_msg(int conn_fd, char *buffer) {
    event_t revents = PERFORM(await, conn_fd, READ | ET);
    if (HANGUP & revents)
        return 0;
    if (!(READ & revents))
        return -1;
    int n_read = recv_syscall_wrapper(conn_fd, buffer, BUF_SIZE, 0);
    if (n_read >= 0)
        buffer[n_read] = 0;
    return n_read;
}

MAKE_SYSCALL_WRAPPER(size_t, send, int, void*, size_t, int);

#ifndef NDEBUG
#define conn_report(msg, ...) \
    threadsafe_printf("[connection %d]: " msg, connection_id, ##__VA_ARGS__)
#else
#define conn_report(msg, ...)
#endif
bool reverse = false;
_Atomic int n_connections = 1;
void *connection_fun(seff_coroutine_t *self, void *_arg) {
#ifndef NDEBUG
    int connection_id = atomic_fetch_add(&n_connections, 1);
#endif

    int conn_fd = (int)(uintptr_t)_arg;
    conn_report("Established connection to client\n");
    char msg_buffer[BUF_SIZE];

    while (true) {
        int n_read = await_msg(conn_fd, msg_buffer);
        if (n_read == 0) {
            conn_report("Connection closed by client\n");
            break;
        } else if (n_read < 0) {
            conn_report("Connection terminated by error\n");
            break;
        }
        conn_report("Message from client: %s", msg_buffer);

        if (reverse) {
            for (int i = 0; i < (n_read - 1) / 2; i++) {
                char tmp = msg_buffer[i];
                msg_buffer[i] = msg_buffer[n_read - 2 - i];
                msg_buffer[n_read - 2 - i] = tmp;
            }
        }

        size_t written = send_syscall_wrapper(conn_fd, msg_buffer, n_read, 0);
        if (written != n_read) {
            conn_report("Failed to send message back\n");
        }
    }

    return NULL;
}

void *listener_fun(seff_coroutine_t *self, void *_arg) {
    int socket_fd = (int)(uintptr_t)_arg;
#ifndef NDEBUG
    threadsafe_printf("Listening for connections\n");
#endif

    while (true) {
        int connection_fd = await_connection(socket_fd);
        if (connection_fd == -1) {
#ifndef NDEBUG
            threadsafe_printf("Error while listening for connection -- did the "
                              "OS kill the socket?\n");
#endif
            break;
        }
#ifndef NDEBUG
        threadsafe_printf("Got connection\n");
#endif
        PERFORM(fork, connection_fun, (void *)(uintptr_t)connection_fd);
    }

    return NULL;
}
#define TASK_QUEUE_SIZE 1000

void print_usage(char *self) {
    printf("Usage: %s [--port P] [--threads N] [--reverse]\n", self);
    exit(-1);
}
int main(int argc, char *argv[]) {
    const char *ip = "127.0.0.1";
    int port = 10000;
    int n_threads = 8;

    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0) {
            if (i + 1 >= argc)
                print_usage(argv[0]);
            sscanf(argv[i + 1], "%d", &port);
            i++;
        } else if (strcmp(argv[i], "--threads") == 0) {
            if (i + 1 >= argc)
                print_usage(argv[0]);
            sscanf(argv[i + 1], "%d", &n_threads);
            i++;
        } else if (strcmp(argv[i], "--reverse") == 0) {
            reverse = true;
        } else {
            print_usage(argv[0]);
        }
    }

    int listen_socket_fd = open_tcp_socket(port, ip, true);
    listen(listen_socket_fd, 1024);
    if (listen_socket_fd == -1) {
        printf("Cannot listen on %s:%d\n", ip, port);
        return -1;
    }
    printf("Open socket %d listening on %s:%d\n", listen_socket_fd, ip, port);
    set_nonblock(listen_socket_fd);

    scheduler_t scheduler;
    scheduler_init(&scheduler, n_threads, TASK_QUEUE_SIZE);
    scheduler_schedule(&scheduler, listener_fun, (void *)(uintptr_t)listen_socket_fd, 0);

    scheduler_start(&scheduler);

    threadsafe_puts("Main idling");

    scheduler_join(&scheduler);
    scheduler_destroy(&scheduler);

    return 0;
}
