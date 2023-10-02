#include <stdbool.h>

int threadsafe_puts(const char *msg);

int threadsafe_fputs(const char *msg, FILE *f);

MAKE_SYSCALL_WRAPPER(int, sprintf, char *, const char *, ...);

#define threadsafe_printf(format, ...)                       \
    do {                                                     \
        char buf[100];                                       \
        sprintf_syscall_wrapper(buf, format, ##__VA_ARGS__); \
        threadsafe_fputs(buf, stdout);                       \
    } while (0);

#ifndef NDEBUG
#define deb_log(msg, ...) threadsafe_printf(msg, ##__VA_ARGS__)
#else
#define deb_log(msg, ...)
#endif
