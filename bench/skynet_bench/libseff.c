#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "seff.h"
#include "skynet_common.h"

DEFINE_EFFECT(yield_int, 1, void, { int64_t value; });

#define STACK_SIZE 256

MAKE_SYSCALL_WRAPPER(
    seff_coroutine_t *, seff_coroutine_new_sized, seff_start_fun_t *, void *, size_t);
MAKE_SYSCALL_WRAPPER(void, seff_coroutine_delete, seff_coroutine_t *);

int64_t last_layer = 1;
void *skynet(void *_arg) {
    int64_t num = (int64_t)_arg;
    seff_coroutine_t *children[10];

    if (num >= last_layer) {
        PERFORM(yield_int, num - last_layer);
    } else {
        num *= 10;
        for (size_t i = 0; i < 10; i++) {
            children[i] =
                seff_coroutine_new_sized_syscall_wrapper(skynet, (void *)(num + i), STACK_SIZE);
        }
        size_t finished = 0;
        while (finished < 10) {
            for (size_t i = 0; i < 10; i++) {
                if (children[i]->state != FINISHED) {
                    seff_handle(children[i], NULL, 0);
                }
                if (children[i]->state == FINISHED) {
                    finished++;
                }
            }
        }
        for (size_t i = 0; i < 10; i++) {
            seff_coroutine_delete_syscall_wrapper(children[i]);
        }
    }

    return NULL;
}

int64_t bench(int n_workers, int depth) {
    if (n_workers > 1) {
        printf("Warning: libseff does not use workers, n_workers is ignored!\n");
    }

    for (int i = 1; i < depth; i++) {
        last_layer *= 10;
    }

    seff_coroutine_t *root = seff_coroutine_new_sized(skynet, (void *)1, STACK_SIZE);

    int64_t total = 0;

    seff_request_t eff = seff_handle(root, NULL, HANDLES(yield_int));
    while (true) {
        switch (eff.effect) {
            CASE_EFFECT(eff, yield_int, {
                total += payload.value;
                break;
            });
            CASE_RETURN(eff, { return total; });
        default:
            assert(false);
        }
        eff = seff_handle(root, NULL, HANDLES(yield_int));
    }

    return total;
}

int main(int argc, char **argv) { return runner(argc, argv, bench, __FILE__); }
