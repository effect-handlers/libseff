#include "seff.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

DEFINE_EFFECT(yield_int, 1, void, { int64_t value; });

void __attribute__((optnone)) syscall(void) {
    // Call to free does not matter, it's just there to force allocate a big segment
    free(NULL);
}

int64_t last_layer = 1;
void *skynet(void *_arg) {
    int64_t num = (int64_t)_arg;
    seff_coroutine_t *children[10];

    if (num >= last_layer) {
        PERFORM(yield_int, num - last_layer);
        syscall();
    } else {
        num *= 10;
        for (size_t i = 0; i < 10; i++) {
            children[i] = seff_coroutine_new(skynet, (void *)(num + i));
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
    }

    return NULL;
}

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

    seff_coroutine_t *root = seff_coroutine_new(skynet, (void *)1);
    int64_t total = 0;

    seff_request_t request = seff_handle(root, NULL, HANDLES(yield_int));
    while (true) {
        switch (request.effect) {
            CASE_EFFECT(request, yield_int, {
                total += payload.value;
                break;
            });
            CASE_RETURN(request, {
                seff_coroutine_delete(root);
                printf("Total: %ld\n", total);
                return 0;
            });
        default:
            assert(false);
        }
        request = seff_handle(root, NULL, HANDLES(yield_int));
    }
}
