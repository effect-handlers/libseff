// Include this file and set these constants:
// - REPS: The number of repetitions to perform the loop
// - BOTTOM_ARR: The size of the array in bottom
// - CORO_SIZE: The size of the coroutine

#include "seff.h"
#include <stdio.h>

int8_t *__attribute_noinline__ bottom(int8_t idx) {
    int8_t arr[BOTTOM_ARR];
    arr[idx] = 0;

    return arr + idx;
}

void *top(seff_coroutine_t *self, void *_arg) {
    int8_t ret = 0;
    for (int i = 0; i < REPS; ++i) {
        int8_t *a = bottom(ret);
        ret = a[ret] + a[0] * ret;
    }

    return (void *)(uintptr_t)ret;
}

int main() {
    seff_coroutine_t *k = seff_coroutine_new_sized(top, NULL, CORO_SIZE);
    seff_resume(k, NULL);

    seff_coroutine_delete(k);
    return 0;
}
