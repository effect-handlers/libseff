#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "seff.h"

// Tests for our own implementation of libgcc split stack mechanism
// Roughly based on https://github.com/ZhiyaoMa98/arm-eabi-seg-stack

#define FORCE_MORESTACK                                     \
    volatile uint32_t pad[PADDING] __attribute__((unused)); \
    for (uint64_t i = 0; i < PADDING; i++) {                \
        pad[i] = i;                                         \
    }

// This will compile to a memset, find something better
#define FORCE_NON_SPLIT volatile uint32_t pad[256] __attribute__((unused)) = {0};

// Check if a large stack frame can be allocated.
__attribute__((optnone, noinline)) uint64_t test_large_frame_alloc(void) {
#define PADDING 256
    FORCE_MORESTACK
#undef PADDING
    return 0;
}

#define PADDING (uint64_t)256 * 256
// 256**4 breaks code generation (clang bug)
// 256**3 breaks comparison (at least in most cases)

// Check if a very large stack frame can be allocated.
__attribute__((optnone, noinline)) uint64_t test_very_large_frame_alloc(void) {
    FORCE_MORESTACK
    return 0;
}

// Check if a non_split stack works fine
__attribute__((optnone, noinline)) uint64_t test_non_split(void) {
    FORCE_NON_SPLIT

    return 0;
}

// Check if stack arguments are properly copied.
__attribute__((optnone, noinline)) uint64_t test_stack_arg(uint32_t reg_arg0, uint32_t reg_arg1,
    uint32_t reg_arg2, uint32_t reg_arg3, uint32_t reg_arg4, uint32_t reg_arg5, uint32_t stk_arg0,
    uint32_t stk_arg1, uint32_t stk_arg2, uint32_t stk_arg3) {
    FORCE_MORESTACK

    return reg_arg0 + reg_arg1 + reg_arg2 + reg_arg3 + reg_arg4 + reg_arg5 + stk_arg0 + stk_arg1 +
           stk_arg2 + stk_arg3;
}

// Check if stack arguments are properly copied when using non_split
__attribute__((optnone, noinline)) uint64_t test_stack_arg_non_split(uint32_t reg_arg0,
    uint32_t reg_arg1, uint32_t reg_arg2, uint32_t reg_arg3, uint32_t reg_arg4, uint32_t reg_arg5,
    uint32_t stk_arg0, uint32_t stk_arg1, uint32_t stk_arg2, uint32_t stk_arg3) {
    FORCE_NON_SPLIT

    return reg_arg0 + reg_arg1 + reg_arg2 + reg_arg3 + reg_arg4 + reg_arg5 + stk_arg0 + stk_arg1 +
           stk_arg2 + stk_arg3;
}

// Check if return values in registers are
// properly passed back to the caller.
__attribute__((optnone, noinline)) uint64_t test_reg_ret(int reg_arg) {
    FORCE_MORESTACK
    return reg_arg;
}

// Check if return values in registers are
// properly passed back to the caller when using non_split
__attribute__((optnone, noinline)) uint64_t test_reg_ret_non_split(int reg_arg) {
    FORCE_NON_SPLIT
    return reg_arg;
}

// A big value that will be returned on the stack
struct StackRetVals {
    uint32_t vals[10];
};

// Comparison operator for these values
__attribute__((optnone, noinline)) bool eq_StackRetVals(
    struct StackRetVals a, struct StackRetVals b) {
    for (int i = 0; i < 10; i++) {
        if (a.vals[i] != b.vals[i])
            return false;
    }
    return true;
}

// Check if returning struct is properly written back
// to the caller's stacklet.
__attribute__((optnone, noinline)) struct StackRetVals test_stack_ret(uint32_t reg_arg0,
    uint32_t reg_arg1, uint32_t reg_arg2, uint32_t reg_arg3, uint32_t reg_arg4, uint32_t reg_arg5,
    uint32_t stk_arg0, uint32_t stk_arg1, uint32_t stk_arg2, uint32_t stk_arg3) {
    FORCE_MORESTACK
    struct StackRetVals vals = {.vals = {stk_arg3, stk_arg2, stk_arg1, stk_arg0, reg_arg5, reg_arg4,
                                    reg_arg3, reg_arg2, reg_arg1, reg_arg0}};
    return vals;
}

// Check if returning struct is properly written back
// to the caller's stacklet, when using non_split
__attribute__((optnone, noinline)) struct StackRetVals test_stack_ret_non_split(uint32_t reg_arg0,
    uint32_t reg_arg1, uint32_t reg_arg2, uint32_t reg_arg3, uint32_t reg_arg4, uint32_t reg_arg5,
    uint32_t stk_arg0, uint32_t stk_arg1, uint32_t stk_arg2, uint32_t stk_arg3) {
    FORCE_NON_SPLIT
    struct StackRetVals vals = {.vals = {stk_arg3, stk_arg2, stk_arg1, stk_arg0, reg_arg5, reg_arg4,
                                    reg_arg3, reg_arg2, reg_arg1, reg_arg0}};
    return vals;
}

// A recursive implementation calculating fibonacci number.
__attribute__((optnone, noinline)) static uint32_t recur_fib(uint32_t x) {
    volatile uint32_t a, b;
    if (x <= 2)
        return 1;
    a = recur_fib(x - 1);
    b = recur_fib(x - 2);
    return a + b;
}

// Wrappers around printf, to avoid using a non_split stack on the main test function
__attribute__((optnone, noinline)) int str_printf(const char *fmt, const char *str) {
    return printf(fmt, str);
}

__attribute__((optnone, noinline)) int int_printf(const char *fmt, uint64_t str) {
    return printf(fmt, str);
}

#define RUN_TEST_ARGS(test, result, ...)                         \
    do {                                                         \
        str_printf("Running test %s ", #test);                   \
        str_printf("with args (%s)\n", #__VA_ARGS__);            \
        uint64_t res = test(__VA_ARGS__);                        \
        if (res != result) {                                     \
            int_printf("    Result %d", res);                    \
            int_printf(" different than expected %d\n", result); \
            return (void *)1;                                    \
        }                                                        \
    } while (0);

#define RUN_TEST_ARGS_STACK(test, result, ...)                        \
    do {                                                              \
        str_printf("Running test %s ", #test);                        \
        str_printf("with args (%s)\n", #__VA_ARGS__);                 \
        struct StackRetVals res = test(__VA_ARGS__);                  \
        if (!eq_StackRetVals(res, result)) {                          \
            int_printf("    Result is different than expected\n", 0); \
            return (void *)1;                                         \
        }                                                             \
    } while (0);

#define RUN_TEST(test)                                      \
    do {                                                    \
        str_printf("Running test %s\n", #test);             \
        uint64_t res = test();                              \
        if (res != 0) {                                     \
            int_printf("    Result %d", res);               \
            int_printf(" different than expected %d\n", 0); \
            return (void *)1;                               \
        }                                                   \
    } while (0);

void *tests(void *arg) {
    RUN_TEST(test_large_frame_alloc)
    RUN_TEST(test_very_large_frame_alloc)
    RUN_TEST(test_non_split)

    RUN_TEST_ARGS(test_stack_arg, 55, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
    RUN_TEST_ARGS(test_stack_arg_non_split, 55, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)

    RUN_TEST_ARGS(test_reg_ret, 42, 42)
    RUN_TEST_ARGS(test_reg_ret_non_split, 42, 42)

    struct StackRetVals ret;
    // Beware of smart code, it may insert a memset or memcopy
    for (int i = 0; i < 10; i++) {
        ret.vals[i] = 10 - i;
    }
    RUN_TEST_ARGS_STACK(test_stack_ret, ret, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
    RUN_TEST_ARGS_STACK(test_stack_ret_non_split, ret, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)

    RUN_TEST_ARGS(recur_fib, 55, 10)
    RUN_TEST_ARGS(recur_fib, 832040, 30)
    RUN_TEST_ARGS(recur_fib, 102334155, 40)

    return NULL;
}

int main(void) {
    // We need to use a coroutine to force the use of segments,
    // otherwise it just uses the system stack
    seff_coroutine_t *k = seff_coroutine_new(tests, NULL);
    seff_request_t result = seff_resume(k, NULL);
    assert(result.effect == EFF_ID(return));
    return (int)(uintptr_t)result.payload;
}
