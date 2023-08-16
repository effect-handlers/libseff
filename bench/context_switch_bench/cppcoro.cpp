#include "context_switch_common.hpp"

#include "cppcoro/async_generator.hpp"
#include "cppcoro/recursive_generator.hpp"
#include "cppcoro/sync_wait.hpp"
#include "cppcoro/task.hpp"

struct awaiter {
    bool is_ready = false;
    std::experimental::coroutine_handle<> self;
    bool await_ready() { return is_ready; }
    void await_suspend(std::experimental::coroutine_handle<> x) { self = x; }
    void await_resume() {}
};
awaiter global_awaiter;

template <size_t padding> cppcoro::async_generator<char *> deep_coroutine(int64_t depth) {
    char arr[padding];
    if (depth == 0) {
        volatile bool loop = true;
        while (loop) {
            co_yield arr;
        }
    } else {
        for
            co_await(auto i : deep_coroutine<padding>(depth - 1)) { co_yield i; }
        co_yield arr;
    }
}

cppcoro::task<> take(
    int iterations, cppcoro::async_generator<char *> &k1, cppcoro::async_generator<char *> &k2) {
    for (size_t i = 0; i < iterations / 2; i++) {
        for
            co_await(auto i : k1) {
                (void)i;
                break;
            }
        for
            co_await(auto i : k2) {
                (void)i;
                break;
            }
    }
}

template <size_t padding> cppcoro::recursive_generator<char *> deep_coroutine_rec(int64_t depth) {
    char arr[padding];
    if (depth == 0) {
        volatile bool loop = true;
        while (loop) {
            co_yield arr;
        }
    } else {
        co_yield deep_coroutine_rec<padding>(depth - 1);
        co_yield arr;
    }
}

void run_benchmark(int iterations, int64_t depth, int padding) {
    if ((true)) {
        auto coroutine_fn = deep_coroutine<0>;
        switch (padding) {
#define X(padding)                              \
    case padding:                               \
        coroutine_fn = deep_coroutine<padding>; \
        break;
            PADDING_SIZES
#undef X
        }
        cppcoro::async_generator<char *> k1 = coroutine_fn(depth);
        cppcoro::async_generator<char *> k2 = coroutine_fn(depth);
        cppcoro::sync_wait(take(iterations, k1, k2));
    } else {
        auto coroutine_fn = deep_coroutine_rec<0>;
        switch (padding) {
#define X(padding)                                  \
    case padding:                                   \
        coroutine_fn = deep_coroutine_rec<padding>; \
        break;
            PADDING_SIZES
#undef X
        }
        cppcoro::recursive_generator<char *> k1 = coroutine_fn(depth);
        cppcoro::recursive_generator<char *> k2 = coroutine_fn(depth);
        cppcoro::recursive_generator<char *>::iterator k1_iter = k1.begin();
        auto k2_iter = k2.begin();
        for (auto i = 0; i < iterations / 2; i++) {
            k1_iter++;
            k2_iter++;
        }
    }
}
