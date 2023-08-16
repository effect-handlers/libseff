#include "memory_common.hpp"

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

void run_benchmark(int instances, int64_t depth, int padding) {
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

    stShareStack_t *share_stack = co_alloc_sharestack(1, 1024 * 128);
    stCoRoutineAttr_t attr;
    attr.stack_size = 0;
    attr.share_stack = share_stack;

    stCoRoutine_t **coroutines;
    coroutines = new stCoRoutine_t *[instances];
    for (auto i = 0; i < instances; i++) {
        co_create(&coroutines[i], &attr, fn, (void *)depth);
        co_resume(coroutines[i]);
    }
    // Removed to make the benchmark run faster
    /*
    for (auto i = 0; i < instances; i++) {
        co_release(coroutines[i]);
    }
    delete[] coroutines;
    */
}
