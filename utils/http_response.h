#include <stddef.h>

/**
 * Build HTTP response on the given buffer
 * Return the number of characters written, or -1 if there was any error
*/
int build_response(char *buffer, size_t buff_size, char *header, char *content_type, void *body, int content_length);
