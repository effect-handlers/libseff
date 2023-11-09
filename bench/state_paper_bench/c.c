#include <stdint.h>
#include <stdio.h>

int64_t value;

void __attribute__((noinline)) put(int64_t number) { value = number; }
int64_t __attribute__((noinline)) get(void) { return value; }

int countdown(int depth) {
    if (depth == 0) {
        for (int i = get(); i > 0; i = get()) {
            put(i - 1);
        }
    } else {
        countdown(depth - 1);
    }
    return 0;
}

int64_t run_benchmark(int iterations, int depth) {
    value = iterations;
    countdown(depth);
    return value;
}

#include "common_main.inc"
