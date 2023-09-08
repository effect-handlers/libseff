#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/**
 * Send HTTP response to the given fd
*/
int send_response(int fd, char *header, char *content_type, void *body, int content_length) {
    const int max_response_size = 262144;
    char response[max_response_size];
    size_t response_length = 0;
    time_t t = time(NULL);
    // DO NOT use localtime
    struct tm *tm = gmtime(&t);
    char s[64];
    strftime(s, sizeof(s), "%c GMT", tm);

    response_length += sprintf(response + response_length, "%s", header);
    response_length += sprintf(response + response_length, "\r\n");
    response_length += sprintf(response + response_length, "Date: %s\r\n", s);
    response_length += sprintf(response + response_length, "Connection: close\r\n");
    response_length += sprintf(response + response_length, "Content-Length: %d\r\n", (content_length > 0) ? content_length + 4 : content_length);
    response_length += sprintf(response + response_length, "Content-Type: %s\r\n", content_type);
    response_length += sprintf(response + response_length, "\r\n");
    if (content_length > 0) {
        response_length += sprintf(response + response_length, "%s", (char*)body);
    	response_length += sprintf(response + response_length, "\r\n\r\n");
    }

    // Send it all!
    int rv = send(fd, response, response_length, MSG_DONTWAIT );
    if (rv == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
        threadsafe_printf("TODO: implement this\n");
    }

    return rv;
}
