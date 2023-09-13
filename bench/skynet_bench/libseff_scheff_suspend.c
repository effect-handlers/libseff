#include "seff.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "scheff.h"

typedef struct {
    _Atomic(bool) ready;
    void *value;
} lwp_t;

wakeup_t lwp_ready(void *_promise) {
    lwp_t *promise = (lwp_t *)_promise;
    if (!promise->ready) {
        printf("promise %p is not ready\n", (void *)promise);
    }
    return (wakeup_t){atomic_load_explicit(&promise->ready, memory_order_relaxed), promise->value};
}

void lwp_fulfill(lwp_t *promise, void *value) {
    printf("fulfilling promise %p\n", (void *)promise);
    promise->value = value;
    atomic_store_explicit(&promise->ready, true, memory_order_release);
}

typedef struct {
    lwp_t result_promise;
    int64_t num;
} skynet_args_t;
// First node id of the last layer
int64_t last_layer = 1;
int64_t total = 0;
void *skynet(seff_coroutine_t *self, void *_arg) {
    skynet_args_t args = *(skynet_args_t *)_arg;
    int64_t num = args.num;
    lwp_t *result_promise = &args.result_promise;

    if (num >= last_layer) {
        lwp_fulfill(result_promise, (void *)(num - last_layer));
    } else {
        int64_t sum = 0;

        int64_t next_layer = num * 10;

        skynet_args_t args[10];
        for (size_t i = 0; i < 10; i++) {
            args[i].result_promise.ready = false;
            args[i].num = next_layer + i;
            scheff_fork(skynet, &args[i]);
        }
        for (size_t i = 0; i < 10; i++) {
            int64_t partial_result =
                (int64_t)(uintptr_t)scheff_suspend(lwp_ready, &args[i].result_promise);
            sum += partial_result;
        }
        lwp_fulfill(result_promise, (void *)sum);
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

    skynet_args_t root_args;
    root_args.result_promise.ready = false;
    root_args.result_promise.value = 1337;
    root_args.num = 1;
    scheff_schedule(&scheduler, skynet, &root_args);

    scheff_run(&scheduler);

    printf("Total: %ld\n", (int64_t)(uintptr_t)root_args.result_promise.value);

#ifndef NDEBUG
    scheff_print_stats(&scheduler);
#endif

    return 0;
}
