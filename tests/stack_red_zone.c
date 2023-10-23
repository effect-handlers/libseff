#include "seff.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *fn(seff_coroutine_t *self, void *arg);
__asm__("fn:"
        "movq $0xFFFFFFFFFFFFFFFF, -0x08(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x10(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x18(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x20(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x28(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x30(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x38(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x40(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x48(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x50(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x58(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x60(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x68(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x70(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x78(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x80(%rsp);"
        "movq $0xFFFFFFFFFFFFFFFF, -0x88(%rsp);"
        // This one touches the canary
        //"movq $0xFFFFFFFFFFFFFFFF, -0x90(%rsp);"

        "movq %rsp, %rax;"
        "ret;");

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new_sized(fn, NULL, 0);
    k->frame_ptr->canary = (void *)0x0;
    seff_resumption_t r = seff_coroutine_start(k);
    seff_resume(r, NULL);

    return (uintptr_t)k->frame_ptr->canary;
}
