
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
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

#include "http_response.h"
#include "net.h"
#include "picohttpparser.h"
#include "scheff.h"
#include "seff.h"

#ifndef NDEBUG
#define deb_log(msg, ...) printf(msg, ##__VA_ARGS__)
#else
#define deb_log(msg, ...)
#endif

#define conn_log(msg, ...) deb_log("[connection %d]: " msg, connection_id, ##__VA_ARGS__)

const char *RESPONSE_TEXT =
    "CHAPTER I. Down the Rabbit-Hole  Alice was beginning to get very tired of sitting by her "
    "sister on the bank, and of having nothing to do: once or twice she had peeped into the book "
    "her sister was reading, but it had no pictures or conversations in it, <and what is the use "
    "of a book,> thought Alice <without pictures or conversations?> So she was considering in her "
    "own mind (as well as she could, for the hot day made her feel very sleepy and stupid), "
    "whether the pleasure of making a daisy-chain would be worth the trouble of getting up and "
    "picking the daisies, when suddenly a White Rabbit with pink eyes ran close by her. There was "
    "nothing so very remarkable in that; nor did Alice think it so very much out of the way to "
    "hear the Rabbit say to itself, <Oh dear! Oh dear! I shall be late!> (when she thought it over "
    "afterwards, it occurred to her that she ought to have wondered at this, but at the time it "
    "all seemed quite natural); but when the Rabbit actually took a watch out of its "
    "waistcoat-pocket, and looked at it, and then hurried on, Alice started to her feet, for it "
    "flashed across her mind that she had never before seen a rabbit with either a "
    "waistcoat-pocket, or a watch to take out of it, and burning with curiosity, she ran across "
    "the field after it, and fortunately was just in time to see it pop down a large rabbit-hole "
    "under the hedge. In another moment down went Alice after it, never once considering how in "
    "the world she was to get out again. The rabbit-hole went straight on like a tunnel for some "
    "way, and then dipped suddenly down, so suddenly that Alice had not a moment to think about "
    "stopping herself before she found herself falling down a very deep well. Either the well was "
    "very deep, or she fell very slowly, for she had plenty of time as she went down to look about "
    "her and to wonder what was going to happen next. First, she tried to look down and make out "
    "what she was coming to, but it was too dark to see anything; then she looked at the sides of "
    "the well, and noticed that they were filled with cupboards......";

MAKE_SYSCALL_WRAPPER(int, build_response, char *, size_t, char *, char *, void *, int);
MAKE_SYSCALL_WRAPPER(void *, calloc, size_t, size_t);
MAKE_SYSCALL_WRAPPER(int, close, int);
MAKE_SYSCALL_WRAPPER(void, free, void *);
MAKE_SYSCALL_WRAPPER(int, strncmp, const char *, const char *, size_t);

// phr_parse_request has too many arguments, this is just a workaround to make it easily wrappable
typedef struct {
    const char *buf_start;
    size_t len;
    const char **method;
    size_t *method_len;
    const char **path;
    size_t *path_len;
    int *minor_version;
    struct phr_header *headers;
    size_t *num_headers;
    size_t last_len;
} phr_parse_args;

int phr_structed(phr_parse_args *args) {
    return phr_parse_request(args->buf_start, args->len, args->method, args->method_len, args->path,
        args->path_len, args->minor_version, args->headers, args->num_headers, args->last_len);
}

MAKE_SYSCALL_WRAPPER(int, phr_structed, phr_parse_args *);

#define BUF_SIZE 64 * 1024
#define MAX_HEADERS 100
_Atomic int n_connections = 1;
void *connection_fun(void *_arg) {
#ifndef NDEBUG
    int connection_id = atomic_fetch_add(&n_connections, 1);
#endif
    conn_log("Established connection to client\n");

    int conn_fd = (int)(uintptr_t)_arg;
    char msg_buffer[BUF_SIZE];
    char response_buffer[BUF_SIZE];

    while (1) {
        // While every new request comes, and the client doesn't close the connection
        const char *method;
        size_t method_len;
        const char *path;
        size_t path_len;
        int minor_version;
        struct phr_header headers[MAX_HEADERS];
        size_t num_headers;
        size_t buflen = 0;
        size_t prevbuflen = 0;
        int parse_ret;

        phr_parse_args args;
        args.buf_start = msg_buffer;
        args.len = buflen;
        args.method = &method;
        args.method_len = &method_len;
        args.path = &path;
        args.path_len = &path_len;
        args.minor_version = &minor_version;
        args.headers = headers;
        args.num_headers = &num_headers;
        args.last_len = prevbuflen;

        while (1) {
            // While the request hasn't been received completely
            int n_read = await_recv(conn_fd, msg_buffer + buflen, BUF_SIZE - buflen);
            if (n_read == 0) {
                conn_log("Connection closed by client\n");
                close_syscall_wrapper(conn_fd);
                return NULL;
            } else if (n_read < 0) {
                conn_log("Connection terminated by error\n");
                close_syscall_wrapper(conn_fd);
                return NULL;
            } else {
                conn_log("Message from client: %s", msg_buffer);
                prevbuflen = buflen;
                buflen += n_read;

                // We must set it before every new call, it's used as two different things
                // internally
                num_headers = MAX_HEADERS;
                args.len = buflen;
                args.last_len = prevbuflen;
                parse_ret = phr_structed_syscall_wrapper(&args);

                if (parse_ret > 0) {
                    /* successfully parsed the request */
                    break;
                } else if (parse_ret == -1) {
                    conn_log("Wrong formatted HTTP message\n");
                    break;
                }
                /* request is incomplete, continue the loop */
                assert(parse_ret == -2);
                if (buflen == BUF_SIZE) {
                    conn_log("Not enough size on the buffer\n");
                    break;
                }
            }
        }

        if (parse_ret > 0) {
            conn_log("server: got method %.*s path %.*s\n", (int)method_len, method, (int)path_len,
                path);
            if (strncmp_syscall_wrapper("/", path, path_len) == 0) {
                int ret = build_response_syscall_wrapper(response_buffer, BUF_SIZE,
                    "HTTP/1.1 200 OK", "text/plain", (void *)RESPONSE_TEXT, strlen(RESPONSE_TEXT));
                if (ret >= 0) {
                    await_send_all(conn_fd, response_buffer, ret);
                } else {
                    conn_log("Couldn't build a response\n");
                }
            } else if (strncmp_syscall_wrapper("/prof", path, path_len) == 0) {
                // TODO actually send the prof values as the reply
                int ret = build_response_syscall_wrapper(
                    response_buffer, BUF_SIZE, "HTTP/1.1 200 OK", "text/plain", NULL, 0);
                if (ret >= 0) {
                    await_send_all(conn_fd, response_buffer, ret);
                } else {
                    conn_log("Couldn't build a response\n");
                }
            } else {
                conn_log("Path %.*s not found\n", (int)path_len, path);
                int ret = build_response_syscall_wrapper(
                    response_buffer, BUF_SIZE, "HTTP/1.1 404 Not Found", "text/plain", NULL, 0);
                if (ret >= 0) {
                    await_send_all(conn_fd, response_buffer, ret);
                } else {
                    conn_log("Couldn't build a response\n");
                }
            }
        }
    }

    // this shouldnt happen
    exit(1);
    return NULL;
}

typedef struct {
    const char *ip;
    const char *port;
} listener_args;

void *listener_fun(void *_arg) {
    listener_args *args = (listener_args *)_arg;
    int socket_fd = listen_tcp_socket(args->ip, args->port, true, true, true, 1024);
    if (socket_fd == -1) {
        printf("Cannot listen on %s:%s\n", args->ip, args->port);
        return (void *)-1;
    }
    printf("Open socket %d listening on %s:%s\n", socket_fd, args->ip, args->port);

    deb_log("Listening for connections\n");

    while (1) {
        int connection_fd = await_accept4(socket_fd);
        if (connection_fd == -1) {
            deb_log("Error while listening for connection -- did the "
                    "OS kill the socket?\n");
            break;
        }
        deb_log("Got connection\n");
        scheff_fork(connection_fun, (void *)(uintptr_t)connection_fd);
    }

    close_syscall_wrapper(socket_fd);

    return NULL;
}

// This is an over estimate, but a proper scheduler should probably have a growable queue
#define TASK_QUEUE_SIZE 10000

void print_usage(char *self) {
    printf("Usage: %s [--port P]\n", self);
    exit(-1);
}

int main(int argc, char *argv[]) {
    const char *ip = "127.0.0.1";
    const char *port = "8082";
    int n_threads = 8;

    const char *s = getenv("LIBSEFF_THREADS");

    if (s != NULL) {
        sscanf(s, "%d", &n_threads);
    }

    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0) {
            if (i + 1 >= argc)
                print_usage(argv[0]);
            port = argv[i + 1];
            i++;
        } else {
            print_usage(argv[0]);
        }
    }

    scheff_t scheduler;
    scheff_init(&scheduler, n_threads);
    printf("Initialized scheduler with %d threads\n", n_threads);

    listener_args l_args;
    l_args.ip = ip;
    l_args.port = port;
    for (int i = 0; i < n_threads; ++i) {
        // schedule a listener function to each thread
        // TODO these listeners can be moved around, should we be able to fix them to a particular
        // thread?
        if (!scheff_schedule(&scheduler, listener_fun, (void *)&l_args)) {
            printf("Something went wrong when scheduling the main task!\n");
            exit(1);
        }
    }

    scheff_run(&scheduler);

#ifndef NDEBUG
    scheff_print_stats(&scheduler);
#endif
    return 0;
}
