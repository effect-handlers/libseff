#include <stdbool.h>

/**
 * Prepares a TCP socket to listen to connections.
 *
 * - ip: the ip to listen to
 * - port: the port, as a const char *
 * - non_blocking, reuse_address, and reuse_port: flags for different attributes
 * - listen_queue_size: the amount of connections to queue up waiting to be accepted
*/
int listen_tcp_socket(const char *ip, const char *port, bool non_blocking, bool reuse_address, bool reuse_port, int listen_queue_size);
