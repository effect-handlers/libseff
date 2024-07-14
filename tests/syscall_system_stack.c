#include "seff.h"
#include <stdio.h>

void hello(void) {
    seff_coroutine_t *coroutine_ptr = seff_current_coroutine();
    char *self = coroutine_ptr ? "a coroutine" : "the top-level";
    printf("Hello from %s!\n", self);
}

MAKE_SYSCALL_WRAPPER(void, hello, void);

void *coroutine(void *arg) {
    hello_syscall_wrapper();
    return NULL;
}

int main(void) {
    hello_syscall_wrapper();
    seff_coroutine_t *k = seff_coroutine_new_sized(coroutine, NULL, 128);
    puts("Created coroutine");
    seff_resume_handling_all(k, NULL);
    seff_coroutine_delete(k);
}
