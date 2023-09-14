#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "atomic.h"
#include "scheff.h"
#include "seff.h"

#define BRANCHING_FACTOR 10

#define EMPTY_PROMISE NULL
#define FULFILLED (void *)0xffffffffffffffff

typedef struct {
    _Atomic(bool) locked;
    struct scheff_waker_t *state;
    int64_t result;
    int64_t id;
} skynet_promise_t;

void fulfill(skynet_promise_t *promise, int64_t result) {
    struct scheff_waker_t *previous_state;
    SPINLOCK(&promise->locked, {
        previous_state = promise->state;
        promise->state = FULFILLED;
        promise->result = result;
    });
    if (previous_state != EMPTY_PROMISE && previous_state != FULFILLED) {
        scheff_wake(previous_state, false);
    }
}

bool must_sleep(struct scheff_waker_t *waker, void *_promise) {
    skynet_promise_t *promise = (skynet_promise_t *)_promise;
    struct scheff_waker_t *state;
    SPINLOCK(&promise->locked, {
        state = promise->state;
        if (state == EMPTY_PROMISE) {
            promise->state = waker;
        }
    });
    return state == EMPTY_PROMISE;
}

void init_promise(skynet_promise_t *promise, int64_t id) {
    promise->state = EMPTY_PROMISE;
    promise->id = id;
    RELAXED(store, &promise->locked, false);
}

int64_t await_promise(skynet_promise_t *promise) {
    struct scheff_waker_t *state;
    SPINLOCK(&promise->locked, { state = promise->state; });
    if (state != FULFILLED)
        scheff_sleep(must_sleep, promise);
    return promise->result;
}

typedef struct {
    skynet_promise_t promise;
    int64_t num;
} skynet_args_t;
// First node id of the last layer
int64_t last_layer = 1;
int64_t total = 0;
void *skynet(seff_coroutine_t *self, void *_arg) {
    skynet_args_t *args = (skynet_args_t *)_arg;
    skynet_promise_t *promise = &args->promise;
    int64_t num = args->num;

    if (num >= last_layer) {
        fulfill(promise, num - last_layer);
    } else {
        int64_t sum = 0;

        int64_t next_layer = num * BRANCHING_FACTOR;

        skynet_args_t child_args[BRANCHING_FACTOR];
        for (size_t i = 0; i < BRANCHING_FACTOR; i++) {
            child_args[i].num = next_layer + i;
            init_promise(&child_args[i].promise, next_layer + i);

            scheff_fork(skynet, &child_args[i]);
        }
        for (size_t i = 0; i < BRANCHING_FACTOR; i++) {
            sum += await_promise(&child_args[i].promise);
        }
        fulfill(promise, sum);
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
        last_layer *= BRANCHING_FACTOR;
    }

    scheff_t scheduler;
    scheff_init(&scheduler, n_workers);

    skynet_args_t root_args;
    init_promise(&root_args.promise, 1);
    root_args.num = 1;
    scheff_schedule(&scheduler, skynet, &root_args);

    scheff_run(&scheduler);

    printf("Total: %ld\n", root_args.promise.result);

#ifndef NDEBUG
    scheff_print_stats(&scheduler);
#endif

    return 0;
}
