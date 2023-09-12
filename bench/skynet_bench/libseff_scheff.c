#include "seff.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "scheff.h"

// First node id of the last layer
typedef struct {
    int64_t num;
    future_t result;
} skynet_args;

int64_t last_layer = 1;
void *skynet(seff_coroutine_t *self, void *_arg) {
    skynet_args *args = (skynet_args *)_arg;
    int64_t num = args->num;
    future_t *fut = &args->result;

    if (num >= last_layer) {
        scheff_fulfill(fut, (void *)(uintptr_t)(num - last_layer));
    } else {
        int64_t sum = 0;

        num *= 10;

        skynet_args children[10];
        for (size_t i = 0; i < 10; i++) {
            children[i] = (skynet_args){num + i, new_future()};
            scheff_fork(skynet, &children[i]);
        }
        for (size_t i = 0; i < 10; i++) {
            int64_t partial_result = (int64_t)(uintptr_t)scheff_await(&children[i].result);
            sum += partial_result;
        }
        scheff_fulfill(fut, (void *)(uintptr_t)sum);
    }
    return NULL;
}

void print_usage(char *self) {
    printf("Usage: %s [--depth M] [--threads N]\n", self);
    exit(-1);
}

int main(int argc, char **argv) {
    int n_workers = 8;
    int depth = 7;

    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--threads") == 0) {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
            }
            int res = sscanf(argv[i + 1], "%d", &n_workers);
            if (res <= 0 || n_workers <= 0) {
                print_usage(argv[0]);
            }
            i++;
        } else if (strcmp(argv[i], "--depth") == 0) {
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

    scheff_t scheduler;
    scheff_init(&scheduler, n_workers);

    skynet_args root_args = (skynet_args){1, new_future()};
    scheff_schedule(&scheduler, skynet, &root_args);

    scheff_run(&scheduler);

    printf("Total: %ld\n", (int64_t)root_args.result.result);

#ifndef NDEBUG
    scheff_print_stats(&scheduler);
#endif

    return 0;
}
