#include "seff.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

DEFINE_EFFECT(yield_int, 1, void, { int64_t value; });

#define STACK_SIZE 256

MAKE_SYSCALL_WRAPPER(
    seff_coroutine_t *, seff_coroutine_new_sized, seff_start_fun_t *, void *, size_t);
MAKE_SYSCALL_WRAPPER(void, seff_coroutine_delete, seff_coroutine_t *);

int64_t last_layer = 1;
void *skynet(seff_coroutine_t *self, void *_arg) {
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

int64_t total;

void print_usage(char *self) {
    printf("Usage: %s [--depth M]\n", self);
    exit(-1);
}

int main(int argc, char **argv) {
    int depth = 7;

    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--depth") == 0) {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
            }
            int res = sscanf(argv[i + 1], "%d", &depth);
            if (res <= 0 || depth <= 0) {
                print_usage(argv[0]);
            }
            i++;
        } else {
            print_usage(argv[0]);
        }
    }

    for (int i = 1; i < depth; i++) {
        last_layer *= 10;
    }

    seff_coroutine_t *root = seff_coroutine_new_sized(skynet, (void *)1, STACK_SIZE);

    total = 0;

    seff_eff_t *eff = seff_handle(root, NULL, HANDLES(yield_int));
    while (root->state != FINISHED) {
        switch (eff->id) {
            CASE_EFFECT(eff, yield_int, {
                total += payload.value;
                break;
            });
        default:
            assert(false);
        }
        eff = seff_handle(root, NULL, HANDLES(yield_int));
    }
    printf("Total: %ld\n", total);
}
