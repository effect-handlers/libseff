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

#include "net.h"
#include "scheff.h"
#include "seff.h"
#include "threadsafe_stdio.h"

#define BUF_SIZE 256

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
        int n_read = await_recv(conn_fd, msg_buffer, BUF_SIZE);
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

        size_t written = await_send_all(conn_fd, msg_buffer, n_read);
        if (written != n_read) {
            conn_report("Failed to send message back\n");
        }
    }

    return NULL;
}

void *listener_fun(seff_coroutine_t *self, void *_arg) {
    int socket_fd = (int)(uintptr_t)_arg;
#ifndef NDEBUG
    threadsafe_puts("Listening for connections\n");
#endif

    while (true) {
        int connection_fd = await_accept4(socket_fd);
        if (connection_fd == -1) {
#ifndef NDEBUG
            threadsafe_puts("Error while listening for connection -- did the "
                            "OS kill the socket?\n");
#endif
            break;
        }
#ifndef NDEBUG
        threadsafe_puts("Got connection\n");
#endif
        scheff_fork(connection_fun, (void *)(uintptr_t)connection_fd);
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
    char *port = "10000";
    int n_threads = 8;

    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0) {
            if (i + 1 >= argc)
                print_usage(argv[0]);
            sscanf(argv[i + 1], "%s", port);
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

    int listen_socket_fd = listen_tcp_socket(ip, port, true, true, false, 1024);

    if (listen_socket_fd == -1) {
        printf("Cannot listen on %s:%s\n", ip, port);
        return -1;
    }

    scheff_t scheduler;
    scheff_init(&scheduler, n_threads);

    if (!scheff_schedule(&scheduler, listener_fun, (void *)(uintptr_t)listen_socket_fd)) {
        printf("Something went wrong when scheduling the main task!\n");
        exit(1);
    }

    scheff_run(&scheduler);

#ifndef NDEBUG
    scheff_print_stats(&scheduler);
#endif
    return 0;
}
