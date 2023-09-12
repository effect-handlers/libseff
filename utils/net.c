#include <net.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#include "scheduler.h"

#ifndef NDEBUG
#define deb_log(msg, ...) \
    printf(msg, ##__VA_ARGS__)
#else
#define deb_log(msg, ...)
#endif

__attribute__((no_split_stack)) int get_errno() {
    // Nothing like defining a macro so a function call looks like a variable
    // so that later on we need to wrap it inside a function again
    return errno;
}

MAKE_SYSCALL_WRAPPER(int, get_errno);
MAKE_SYSCALL_WRAPPER(int, accept4, int, void*, void*, int);
MAKE_SYSCALL_WRAPPER(int, recv, int, void*, size_t, int);
MAKE_SYSCALL_WRAPPER(int, send, int, const void*, size_t, int);

int listen_tcp_socket(const char *ip, const char *port, bool non_blocking, bool reuse_address, bool reuse_port, int listen_queue_size){
    deb_log("Getting listening TCP socket on ip: %s, port: %s\n", ip, port);
    deb_log("  non blocking: %d, reuse address: %d, reuse port: %d\n", non_blocking, reuse_address, reuse_port);

    int listener = -1;     // Listening socket descriptor
    int ret_val;

    // getAddrInfo, this is compatible with IPv6 and should resolve DNS lookups
    struct addrinfo hints;
    struct addrinfo *ai;
    struct addrinfo *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((ret_val = getaddrinfo(ip, port, &hints, &ai)) != 0) {
        deb_log("getaddrinf failed with: %s\n", gai_strerror(ret_val));
        return -1;
    }

    // Try to bind to all of them
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype | (non_blocking ? SOCK_NONBLOCK : 0), p->ai_protocol);
        if (listener < 0) {
            deb_log("Failure to create socket with family: %d, type: %d, protocol: %d\n", p->ai_family, p->ai_socktype | (non_blocking ? SOCK_NONBLOCK : 0), p->ai_protocol);
            continue;
        }

        if (reuse_address){
            deb_log("Reusing address\n");
            int yes = 1;
            setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        }

        if (reuse_port){
            deb_log("Reusing port\n");
            int yes = 1;
            setsockopt(listener, SOL_SOCKET, SO_REUSEPORT , &yes, sizeof(yes));
        }


        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            deb_log("Failure to bind socket\n");
            close(listener);
            continue;
        }

        break;
    }

    // Cleanup
    freeaddrinfo(ai);
    if (p == NULL){
        deb_log("None of the addrinfo succeded, exiting\n");
        return -1;
    }

    // Listen
    deb_log("Setting up listener with %d backlog\n", listen_queue_size);
    if (listen(listener, listen_queue_size) == -1) {
        close(listener);
        return -1;
    }

    return listener;
}


int await_accept4(int socket_fd) {
    // it is assumed that the socket is non blocking
#ifndef NDEBUG
    // assert that the socket is non blocking
    // assert()
#endif
    int n_read = accept4_syscall_wrapper(socket_fd, NULL, NULL, SOCK_NONBLOCK);
    if (n_read >= 0) {
        // accept succesful (happy path)
        return n_read;
    } else {
        int err = get_errno_syscall_wrapper();
        if (err != EAGAIN && err != EWOULDBLOCK) {
            // No point in waiting, something else made it fail
            deb_log("Call to accept4 failed! returning error\n");
            return n_read;
        }
    }

    event_t revents = PERFORM(await, socket_fd, READ | ET);
    if (HANGUP & revents)
        return 0;
    if (!(READ & revents))
        return -1;

    // There must be something here
    return accept4_syscall_wrapper(socket_fd, NULL, NULL, SOCK_NONBLOCK);
}


int await_recv(int conn_fd, char *buffer, size_t bufsz) {
    int n_read = recv_syscall_wrapper(conn_fd, buffer, bufsz, MSG_DONTWAIT);
    if (n_read >= 0) {
        // recv succesful
        // TODO remove this, I don't like it
        buffer[n_read] = 0;
        return n_read;
    } else {
        int err = get_errno_syscall_wrapper();
        if (err != EAGAIN && err != EWOULDBLOCK) {
            // No point in waiting
            deb_log("Call to recv failed! returning error\n");
            return n_read;
        }

    }
    event_t revents = PERFORM(await, conn_fd, READ | ET);
    if (HANGUP & revents)
        return 0;
    if (!(READ & revents))
        return -1;

    // Assume there's data to read
    return recv_syscall_wrapper(conn_fd, buffer, bufsz, MSG_DONTWAIT);
}

int await_send(int conn_fd, const char *buffer, size_t bufsz) {
    int n_read = send_syscall_wrapper(conn_fd, buffer, bufsz, MSG_DONTWAIT);
    if (n_read >= 0) {
        // send succesful
        return n_read;
    } else {
        int err = get_errno_syscall_wrapper();
        if (err != EAGAIN && err != EWOULDBLOCK) {
            // No point in waiting
            deb_log("Call to send failed! returning error\n");
            return n_read;
        }

    }
    event_t revents = PERFORM(await, conn_fd, WRITE);
    if (HANGUP & revents)
        return 0;
    if (!(WRITE & revents))
        return -1;

    // Assume there's space to write
    return send_syscall_wrapper(conn_fd, buffer, bufsz, MSG_DONTWAIT);
}

int await_send_all(int conn_fd, const char *buffer, size_t bufsz){
    int total = 0; // how many bytes we've sent
    int bytesleft = bufsz; // how many we have left to send
    int n;

    while(total < bufsz) {
        n = await_send(conn_fd, buffer+total, bytesleft);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    return n==-1?-1:total; // return -1 on failure, 0 on success
}
