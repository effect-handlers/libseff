#include "seff.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "scheff.h"

// First node id of the last layer
int64_t last_layer = 1;
int64_t total = 0;
void *skynet(seff_coroutine_t *self, void *_arg) {
    int64_t num = (int64_t)(uintptr_t)_arg;

    if (num >= last_layer) {
        return (void *)(uintptr_t)(num - last_layer);
    } else {
        int64_t sum = 0;

        int64_t next_layer = num * 10;

        future_t futures[10];
        for (size_t i = 0; i < 10; i++) {
            scheff_async(skynet, (void *)(next_layer + i), &futures[i]);
        }
        for (size_t i = 0; i < 10; i++) {
            int64_t partial_result = (int64_t)(uintptr_t)scheff_await(&futures[i]);
            sum += partial_result;
        }
        if (num == 1) {
            total = sum;
        }
        return (void *)sum;
    }
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

    scheff_schedule(&scheduler, skynet, (void *)1);

    scheff_run(&scheduler);

    printf("Total: %ld\n", total);

#ifndef NDEBUG
    scheff_print_stats(&scheduler);
#endif

    return 0;
}
