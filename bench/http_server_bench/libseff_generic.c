
#include <arpa/inet.h>
#include <assert.h>
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
#include <errno.h>


#include "picohttpparser.h"
#include "seff.h"
#include "utils/http_response.h"
#include "utils/net.h"

#ifndef NDEBUG
#define deb_log(msg, ...) \
    threadsafe_printf(msg, ##__VA_ARGS__)
#else
#define deb_log(msg, ...)
#endif
#define conn_log(msg, ...) \
    deb_log("[connection %d]: " msg, connection_id, ##__VA_ARGS__)


#define PROF_VAR(a) _Atomic int a
#define PROF_INC(a) atomic_fetch_add_explicit(&a, 1, memory_order_relaxed)
#define PROF_LOAD(a) atomic_load_explicit(&a, memory_order_relaxed)
// Manual for now, maybe we want another compile time flag (-DPROFILE)
// #define PROF_VAR(a)
// #define PROF_INC(a)
// #define PROF_LOAD(a)



PROF_VAR(awaited_msgs);
PROF_VAR(recv_msgs);

PROF_VAR(awaited_conn);
PROF_VAR(recv_conn);

PROF_VAR(incomplete_reqs);

const char* RESPONSE_TEXT = "CHAPTER I. Down the Rabbit-Hole  Alice was beginning to get very tired of sitting by her sister on the bank, and of having nothing to do: once or twice she had peeped into the book her sister was reading, but it had no pictures or conversations in it, <and what is the use of a book,> thought Alice <without pictures or conversations?> So she was considering in her own mind (as well as she could, for the hot day made her feel very sleepy and stupid), whether the pleasure of making a daisy-chain would be worth the trouble of getting up and picking the daisies, when suddenly a White Rabbit with pink eyes ran close by her. There was nothing so very remarkable in that; nor did Alice think it so very much out of the way to hear the Rabbit say to itself, <Oh dear! Oh dear! I shall be late!> (when she thought it over afterwards, it occurred to her that she ought to have wondered at this, but at the time it all seemed quite natural); but when the Rabbit actually took a watch out of its waistcoat-pocket, and looked at it, and then hurried on, Alice started to her feet, for it flashed across her mind that she had never before seen a rabbit with either a waistcoat-pocket, or a watch to take out of it, and burning with curiosity, she ran across the field after it, and fortunately was just in time to see it pop down a large rabbit-hole under the hedge. In another moment down went Alice after it, never once considering how in the world she was to get out again. The rabbit-hole went straight on like a tunnel for some way, and then dipped suddenly down, so suddenly that Alice had not a moment to think about stopping herself before she found herself falling down a very deep well. Either the well was very deep, or she fell very slowly, for she had plenty of time as she went down to look about her and to wonder what was going to happen next. First, she tried to look down and make out what she was coming to, but it was too dark to see anything; then she looked at the sides of the well, and noticed that they were filled with cupboards......";

MAKE_SYSCALL_WRAPPER(int, accept4, int, void*, void*, int);
int await_connection(int socket_fd) {
    int n_read;
    PROF_INC(recv_conn);

    n_read = accept4_syscall_wrapper(socket_fd, NULL, NULL, SOCK_NONBLOCK);
    if (n_read >= 0) {
        // accept succesful
        return n_read;
    } else {
        int err = errno;
        if (err != EAGAIN && err != EWOULDBLOCK) {
            // No point in waiting
            return n_read;
        }
    }
    PROF_INC(awaited_conn);
    event_t revents = PERFORM(await, socket_fd, READ);
    if (HANGUP & revents)
        return 0;
    if (!(READ & revents))
        return -1;

    n_read = accept4_syscall_wrapper(socket_fd, NULL, NULL, 0);

    return n_read;
}

MAKE_SYSCALL_WRAPPER(int, recv, int, void*, size_t, int);

int await_msg(int conn_fd, char *buffer, size_t bufsz) {
    int n_read;
    PROF_INC(recv_msgs);

    n_read = recv_syscall_wrapper(conn_fd, buffer, bufsz, MSG_DONTWAIT);
    if (n_read >= 0) {
        // accept succesful
        buffer[n_read] = 0;
        return n_read;
    } else {
        int err = errno;
        if (err != EAGAIN && err != EWOULDBLOCK) {
            // No point in waiting
            return n_read;
        }

    }
    PROF_INC(awaited_msgs);
    event_t revents = PERFORM(await, conn_fd, READ | ET);
    if (HANGUP & revents)
        return 0;
    if (!(READ & revents))
        return -1;

    n_read = recv_syscall_wrapper(conn_fd, buffer, bufsz, 0);

    return n_read;
}

MAKE_SYSCALL_WRAPPER(size_t, send_response, int, char*, char*, void*, int);


#define BUF_SIZE 64*1024
#define MAX_HEADERS 100
_Atomic int n_connections = 1;
void *connection_fun(seff_coroutine_t *self, void *_arg) {
#ifndef NDEBUG
    int connection_id = atomic_fetch_add(&n_connections, 1);
#endif
    conn_log("Established connection to client\n");

    int conn_fd = (int)(uintptr_t)_arg;
    // TODO improve on this calloc
    char* msg_buffer = calloc(BUF_SIZE, sizeof(char));

    const char* method;
    size_t method_len;
    const char* path;
    size_t path_len;
    int minor_version;
    struct phr_header headers[MAX_HEADERS];
    size_t num_headers;
    size_t buflen = 0;
    size_t prevbuflen = 0;
    int parse_ret;

    while (1) {
        int n_read = await_msg(conn_fd, msg_buffer + buflen, BUF_SIZE - buflen);
        if (n_read == 0) {
            conn_log("Connection closed by client\n");
            return NULL;
        } else if (n_read < 0) {
            conn_log("Connection terminated by error\n");
            return NULL;
        } else {
            conn_log("Message from client: %s", msg_buffer);
            prevbuflen = buflen;
            buflen += n_read;

            // We must set it before every new call, it's used as two different things internally
            num_headers = MAX_HEADERS;
            parse_ret = phr_parse_request(msg_buffer, buflen, &method, &method_len, &path, &path_len,
                                &minor_version, headers, &num_headers, prevbuflen);

            if (parse_ret > 0) {
                /* successfully parsed the request */
                break;
            } else if (parse_ret == -1) {
                conn_log("Wrong formatted HTTP message\n");
                break;
            }
            /* request is incomplete, continue the loop */
            PROF_INC(incomplete_reqs);
            assert(parse_ret == -2);
            if (buflen == BUF_SIZE) {
                conn_log("Not enough size on the buffer\n");
                break;
            }
        }
    }

    if (parse_ret > 0){
        conn_log("server: got method %.*s path %.*s\n", (int)method_len, method, (int)path_len, path);
        if (strncmp("/", path, path_len) == 0){
            send_response_syscall_wrapper(conn_fd, "HTTP/1.1 200 OK", "text/plain", (void*)RESPONSE_TEXT, strlen(RESPONSE_TEXT));
        } else if (strncmp("/prof", path, path_len) == 0){
            threadsafe_printf("awaited_msgs: %d\n", PROF_LOAD(recv_msgs));
            threadsafe_printf("awaited_msgs: %d\n", PROF_LOAD(awaited_msgs));
            threadsafe_printf("awaited_conn: %d\n", PROF_LOAD(recv_conn));
            threadsafe_printf("awaited_conn: %d\n", PROF_LOAD(awaited_conn));
            threadsafe_printf("incomplete_reqs: %d\n", PROF_LOAD(incomplete_reqs));
            send_response_syscall_wrapper(conn_fd, "HTTP/1.1 200 OK", "text/plain", NULL, 0);
        } else {
            conn_log("Path %.*s not found\n", (int)path_len, path);
            send_response_syscall_wrapper(conn_fd, "HTTP/1.1 404 Not Found", "text/plain", NULL, 0);
        }
    }

    close(conn_fd);
    free(msg_buffer);

    return NULL;
}



void *listener_fun(seff_coroutine_t *self, void *_arg) {
    int socket_fd = (int)(uintptr_t)_arg;
    deb_log("Listening for connections\n");

    while (true) {
        int connection_fd = await_connection(socket_fd);
        if (connection_fd == -1) {
            deb_log("Error while listening for connection -- did the "
                              "OS kill the socket?\n");
            break;
        }
        deb_log("Got connection\n");
        PERFORM(fork, connection_fun, (void *)(uintptr_t)connection_fd);
    }

    return NULL;
}
#define TASK_QUEUE_SIZE 10000

void print_usage(char *self) {
    printf("Usage: %s [--port P] [--reverse]\n", self);
    exit(-1);
}

int main(int argc, char *argv[]) {
    const char *ip = "127.0.0.1";
    int port = 8082;
    int n_threads = 8;

    const char* s = getenv("LIBSEFF_THREADS");

    if (s != NULL){
        sscanf(s, "%d", &n_threads);
    }

    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0) {
            if (i + 1 >= argc)
                print_usage(argv[0]);
            sscanf(argv[i + 1], "%d", &port);
            i++;
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
    printf("Initialized scheduler with %d threads\n", n_threads);
    scheduler_schedule(&scheduler, listener_fun, (void *)(uintptr_t)listen_socket_fd, 23);

    scheduler_start(&scheduler);

    threadsafe_puts("Main idling");

    scheduler_join(&scheduler);
    scheduler_destroy(&scheduler);

    close(listen_socket_fd);

    return 0;
}
