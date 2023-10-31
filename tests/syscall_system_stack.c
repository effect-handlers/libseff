#include "seff.h"
#include <stdio.h>

void hello(void) { printf("Hello world! I am a %s!\n", "coroutine"); }

MAKE_SYSCALL_WRAPPER(void, hello, void);

void *coroutine(void *arg) {
    hello_syscall_wrapper();
    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new_sized(coroutine, NULL, 128);
    puts("Created coroutine");
    seff_resume(k, NULL);
    seff_coroutine_delete(k);
}
