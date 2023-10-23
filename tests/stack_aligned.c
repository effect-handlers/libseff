#include "seff.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *fn(seff_coroutine_t *self, void *arg);
__asm__("fn:"
        "movq %rsp, %rax;"
        "ret;");

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(fn, NULL);
    seff_resumption_t r = seff_coroutine_start(k);
    void *rsp = seff_resume(r, NULL);

    // As soon as a function enters the stack is 16 byte misaligned by 8
    return ((uintptr_t)rsp + 8) % 16;
}
