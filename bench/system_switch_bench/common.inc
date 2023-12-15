// Include this file and set these constants:
// - REPS: The number of repetitions to perform the loop
// - REPS_PUTS: The number of repetitions to perform the puts loop
// - CORO_SIZE: The size of the coroutine
// - COROUTINE: Whether to use coroutines or not

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef COROUTINE
#include "seff.h"
MAKE_SYSCALL_WRAPPER(void, free, void *);
MAKE_SYSCALL_WRAPPER(int, puts, const char *);
#define BOTTOM_FN(MULTS) bottom_##MULTS##_syscall_wrapper
#define MAKE_WRAPPER(MULTS) MAKE_SYSCALL_WRAPPER(int8_t *, bottom_##MULTS);
#define BOTTOM_PUTS puts_syscall_wrapper
#else
#define BOTTOM_FN(MULTS) bottom_##MULTS
#define MAKE_WRAPPER(MULTS)
#define BOTTOM_PUTS puts
#endif

#ifdef DEALLOC
#define DEALLOC_HEADER seff_coroutine_t *curr = seff_current_coroutine();
#define DEALLOC_BODY                                     \
    seff_frame_ptr_t next_frame = curr->frame_ptr->next; \
    curr->frame_ptr->next = NULL;                        \
    while (next_frame != NULL) {                         \
        seff_frame_ptr_t old = next_frame;               \
        next_frame = next_frame->next;                   \
        free_syscall_wrapper(old);                       \
    }
#else
#define DEALLOC_HEADER
#define DEALLOC_BODY
#endif

int8_t v;
double x;

#define FUNCTIONS(MULTS)                                                                      \
    int8_t *__attribute_noinline__ bottom_##MULTS() {                                         \
                                                                                              \
        for (int i = 0; i < MULTS; ++i) {                                                     \
            x = x * 3.0;                                                                      \
            /* asm volatile("sqrtsd %0, %0 \n" : "+x"(x) : :);*/                              \
        }                                                                                     \
                                                                                              \
        /* Avoids inlining */                                                                 \
        __asm__("" : "=o"(v) : "o"(v));                                                       \
        return (int8_t *)&v;                                                                  \
    }                                                                                         \
    MAKE_WRAPPER(MULTS)                                                                       \
                                                                                              \
    void *__attribute_noinline__ top_##MULTS(void *_arg) {                                    \
        DEALLOC_HEADER                                                                        \
        for (int i = 0; i < REPS; ++i) {                                                      \
            int8_t *a = BOTTOM_FN(MULTS)(); /* So the result from bottom is actually "used"*/ \
            __asm__("" : "=r"(a) : "r"(a));                                                   \
                                                                                              \
            DEALLOC_BODY                                                                      \
        }                                                                                     \
                                                                                              \
        return NULL;                                                                          \
    }

FUNCTIONS(0)
FUNCTIONS(1)
FUNCTIONS(2)
FUNCTIONS(3)
FUNCTIONS(4)
FUNCTIONS(5)
FUNCTIONS(6)
FUNCTIONS(7)
FUNCTIONS(8)
FUNCTIONS(9)
FUNCTIONS(10)
FUNCTIONS(11)
FUNCTIONS(12)
FUNCTIONS(13)
FUNCTIONS(14)
FUNCTIONS(15)
FUNCTIONS(16)
FUNCTIONS(17)
FUNCTIONS(18)
FUNCTIONS(19)
FUNCTIONS(20)

void *__attribute_noinline__ top_puts(void *_arg) {
    DEALLOC_HEADER
    for (int i = 0; i < REPS_PUTS; ++i) {
        BOTTOM_PUTS("Hello World!");
        // __asm__("" : "=r"(a) : "r"(a));

        DEALLOC_BODY
    }

    return NULL;
}

int __attribute_noinline__ runner(void *(*top)(void *)) {
#ifdef COROUTINE
    seff_coroutine_t *k = seff_coroutine_new_sized(top, NULL, CORO_SIZE);
    void *a = seff_resume(k, NULL).payload;

    seff_coroutine_delete(k);
#else
    void *a = top(NULL);

#endif
    return (uintptr_t)a;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <multiplications or puts>\n", argv[0]);
        return 1;
    }

    void *(*top)(void *);

    if (strcmp(argv[1], "puts") == 0) {
        top = top_puts;
    } else {
        int mults = 0;
        sscanf(argv[1], "%d", &mults);
        x = 4.0;

        switch (mults) {
#define CASE(X)        \
    case X:            \
        top = top_##X; \
        break;
            CASE(0)
            CASE(1)
            CASE(2)
            CASE(3)
            CASE(4)
            CASE(5)
            CASE(6)
            CASE(7)
            CASE(8)
            CASE(9)
            CASE(10)
            CASE(11)
            CASE(12)
            CASE(13)
            CASE(14)
            CASE(15)
            CASE(16)
            CASE(17)
            CASE(18)
            CASE(19)
            CASE(20)
#undef CASE
        default:
            printf("Multiplications must be between 0 and 20\n");
            return 1;
        }
    }

    return runner(top);
}
