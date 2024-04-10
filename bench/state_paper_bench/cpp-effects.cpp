#include <cstdio>

#include "cpp-effects/clause-modifiers.h"
#include "cpp-effects/cpp-effects.h"

namespace eff = cpp_effects;

struct Put : eff::command<> {
    int64_t newState;
};
struct Get : eff::command<int64_t> {};

int64_t value;

template <typename T> // the type of the handled computation
class State : public eff::handler<T, T, Put, Get> {
  private:
    T handle_command(Get, eff::resumption<typename Get::resumption_type<T>> r) override {
        return std::move(r).tail_resume(value);
    }
    T handle_command(Put p, eff::resumption<typename Put::resumption_type<T>> r) override {
        value = p.newState;
        return std::move(r).tail_resume();
    }
    T handle_return(T v) override { return v; }
};

void put(int64_t s) { eff::invoke_command(Put{{}, s}); }
int64_t get() { return eff::invoke_command(Get{}); }

struct Error : eff::command<> {};
class Catch : public eff::handler<void, void, Error> {
    void handle_return() final override {}
    void handle_command(Error, eff::resumption<void()>) final override {}
};

int stateful(int depth) {
    if (depth == 0) {
        for (int i = get(); i > 0; i = get()) {
            put(i - 1);
        }
    } else {
        eff::handle<Catch>([=]() { return stateful(depth - 1); });
    }
    return 0;
}

int64_t run_benchmark(int iterations, int depth) {
    value = iterations;
    eff::handle<State<int>>([=]() { return stateful(depth); });
    return value;
}

#include "common_main.inc"
