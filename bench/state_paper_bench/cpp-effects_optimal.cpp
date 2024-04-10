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
class State : public eff::flat_handler<T, eff::plain<Put>, eff::plain<Get>> {
  private:
    int64_t handle_command(Get) final override { return value; }
    void handle_command(Put p) final override {
        value = p.newState;
        return;
    }
};

void put(eff::handler_ref ref, int64_t s) {
    eff::static_invoke_command<State<int>>(ref, Put{{}, s});
}
int64_t get(eff::handler_ref ref) { return eff::static_invoke_command<State<int>>(ref, Get{}); }

struct Error : eff::command<> {};
class Catch : public eff::handler<void, void, Error> {
    void handle_return() final override {}
    void handle_command(Error, eff::resumption<void()>) final override {}
};

int stateful(int depth) {
    if (depth == 0) {
        eff::handler_ref get_ref = eff::find_handler<Get>();
        eff::handler_ref put_ref = eff::find_handler<Put>();
        for (int i = get(get_ref); i > 0; i = get(get_ref)) {
            put(put_ref, i - 1);
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
