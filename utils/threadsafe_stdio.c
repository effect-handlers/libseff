#include <stdatomic.h>

#include "mvar.h"
#include "threadsafe_stdio.h"

// Start the stdout_lock with a value
static mvar_t stdout_lock = MVAR_FILLED((void *)0x1);

MAKE_SYSCALL_WRAPPER(int, puts, const char *);

int threadsafe_puts(const char *msg) {
    void *lock = mvar_take(&stdout_lock);
    int ret = puts_syscall_wrapper(msg);
    mvar_put(&stdout_lock, lock);
    return ret;
}

MAKE_SYSCALL_WRAPPER(int, fputs, const char *, FILE *);

int threadsafe_fputs(const char *msg, FILE *f) {
    void *lock = mvar_take(&stdout_lock);
    int ret = fputs_syscall_wrapper(msg, f);
    mvar_put(&stdout_lock, lock);
    return ret;
}
