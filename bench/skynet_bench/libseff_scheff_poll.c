#include "seff.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "atomic.h"
#include "scheff.h"
#include "skynet_common.h"

typedef struct {
    _Atomic(bool) ready;
    int64_t value;
} lwp_t;

bool lwp_ready(void *_promise) {
    lwp_t *promise = (lwp_t *)_promise;
    return RELAXED(load, &promise->ready);
}

void lwp_fulfill(lwp_t *promise, int64_t value) {
    promise->value = value;
    RELAXED(store, &promise->ready, true);
}

typedef struct {
    lwp_t result_promise;
    int64_t num;
} skynet_args_t;

// First node id of the last layer
int64_t last_layer = 1;

void *skynet(void *_arg) {
    int64_t num = ((skynet_args_t *)_arg)->num;
    lwp_t *result_promise = &((skynet_args_t *)_arg)->result_promise;

    if (num >= last_layer) {
        lwp_fulfill(result_promise, num - last_layer);
    } else {
        int64_t sum = 0;

        int64_t next_layer = num * BRANCHING_FACTOR;

        skynet_args_t args[10];
        for (size_t i = 0; i < 10; i++) {
            args[i].result_promise.ready = false;
            args[i].num = next_layer + i;
            scheff_fork(skynet, &args[i]);
        }
        for (size_t i = 0; i < 10; i++) {
            scheff_poll(lwp_ready, &args[i].result_promise);
            sum += args[i].result_promise.value;
        }
        lwp_fulfill(result_promise, sum);
    }
    return NULL;
}

int64_t bench(int n_workers, int depth) {
    for (int i = 1; i < depth; i++) {
        last_layer *= 10;
    }

    scheff_t scheduler;
    scheff_init(&scheduler, n_workers);

    skynet_args_t root_args;
    root_args.result_promise.ready = false;
    root_args.num = 1;
    if (!scheff_schedule(&scheduler, skynet, &root_args)) {
        printf("Something went wrong when scheduling the main task!\n");
        exit(1);
    }

    scheff_run(&scheduler);

#ifndef NDEBUG
    scheff_print_stats(&scheduler);
#endif

    return (int64_t)(uintptr_t)root_args.result_promise.value;
}

int main(int argc, char **argv) { return runner(argc, argv, bench, __FILE__); }
