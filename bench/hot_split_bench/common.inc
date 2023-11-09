// Include this file and set these constants:
// - REPS: The number of repetitions to perform the loop
// - BOTTOM_ARR: The size of the array in bottom
// - CORO_SIZE: The size of the coroutine
// - COROUTINE: Whether to use coroutines or not

#include <stddef.h>
#include <stdint.h>

#ifdef COROUTINE
#include "seff.h"
#endif

int8_t *__attribute_noinline__ bottom(int8_t idx) {
    int8_t arr[BOTTOM_ARR];
    arr[idx] = 0;

    return arr + idx;
}

void *__attribute_noinline__ top(void *_arg) {
    int8_t ret = 0;

#ifdef DEALLOC
    seff_coroutine_t *curr = seff_current_coroutine();
#endif

    for (int i = 0; i < REPS; ++i) {
        int8_t *a = bottom(ret);

#ifdef DEALLOC
        seff_frame_ptr_t next_frame = curr->frame_ptr->next;
        curr->frame_ptr->next = NULL;
        while (next_frame != NULL) {
            seff_frame_ptr_t old = next_frame;
            next_frame = next_frame->next;
            free(old);
        }
#endif

        ret = a[ret] + a[0] * ret;
    }

    return (void *)(uintptr_t)ret;
}

int main() {
#ifdef COROUTINE
    seff_coroutine_t *k = seff_coroutine_new_sized(top, NULL, CORO_SIZE);
    seff_resume(k, NULL);

    seff_coroutine_delete(k);
    return 0;
#else
    void *a = top(NULL);

    return (uintptr_t)a;
#endif
}
