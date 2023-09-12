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

#ifndef NDEBUG
#define deb_log(msg, ...) \
    printf(msg, ##__VA_ARGS__)
#else
#define deb_log(msg, ...)
#endif

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
