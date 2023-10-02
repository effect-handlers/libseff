#include <stdatomic.h>

#include "mvar.h"
#include "threadsafe_stdio.h"

static struct atomic_flag initialized;
static mvar_t stdout_lock;

bool init_locks() {
    bool _initialized = atomic_flag_test_and_set(&initialized);
    if (!_initialized) {
        mvar_init(&stdout_lock);
        // We don't really care about the value, as long it's not NULL
        mvar_put(&stdout_lock, (void *)0x1);
        return true;
    }
    return false;
}

MAKE_SYSCALL_WRAPPER(int, puts, const char *);

int threadsafe_puts(const char *msg) {
    void *lock = mvar_take(&stdout_lock);
    int ret = puts_syscall_wrapper(msg);
    mvar_put(&stdout_lock, lock);
    return ret;
}
