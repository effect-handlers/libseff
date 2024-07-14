#include "memory_common.hpp"

#include "seff.h"

template <size_t padding> void *deep_coroutine(void *arg) {
    char arr[padding];
    int64_t depth = (int64_t)arg;
    if (depth == 0) {
        seff_coroutine_t *self = seff_current_coroutine();
        volatile bool loop = true;
        while (loop) {
            seff_yield(self, 0, nullptr);
        }
        return arr;
    } else {
        deep_coroutine<padding>((void *)(depth - 1));
        return arr;
    }
}

void run_benchmark(int instances, int64_t depth, int padding) {
    seff_start_fun_t *fn;
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

    seff_coroutine_t **coroutines;
    coroutines = new seff_coroutine_t *[instances];
    for (auto i = 0; i < instances; i++) {
        coroutines[i] = seff_coroutine_new(fn, (void *)depth);
        seff_resume_handling_all(coroutines[i], nullptr);
    }
    // Removed to make the benchmark run faster
    /*
    for (auto i = 0; i < instances; i++) {
        seff_coroutine_delete(coroutines[i]);
    }
    delete[] coroutines;
    */
}
