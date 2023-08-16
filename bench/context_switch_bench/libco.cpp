#include "context_switch_common.hpp"

#include "co_routine.h"

template <size_t padding> void *deep_coroutine(void *arg) {
    char arr[padding];
    int64_t depth = (int64_t)arg;
    if (depth == 0) {
        volatile bool loop = true;
        while (loop) {
            co_yield_ct();
        }
        return arr;
    } else {
        deep_coroutine<padding>((void *)(depth - 1));
        return arr;
    }
}

void run_benchmark(int iterations, int64_t depth, int padding) {
    auto *fn = deep_coroutine<0>;
    switch (padding) {
#define X(n)                    \
    case n:                     \
        fn = deep_coroutine<n>; \
        break;
        PADDING_SIZES
#undef X
    default:
        printf("Incorrect padding\n");
        return;
    }

    stCoRoutine_t *k1;
    stCoRoutine_t *k2;
    stShareStack_t *share_stack = co_alloc_sharestack(1, 1024 * 128);
    stCoRoutineAttr_t attr;
    attr.stack_size = 0;
    attr.share_stack = share_stack;
    co_create(&k1, &attr, fn, (void *)depth);
    co_create(&k2, &attr, fn, (void *)depth);
    for (size_t i = 0; i < iterations / 2; i++) {
        co_resume(k1);
        co_resume(k2);
    }
    co_release(k1);
    co_release(k2);
}
