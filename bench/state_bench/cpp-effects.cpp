#include <cstdio>

#include "cpp-effects/clause-modifiers.h"
#include "cpp-effects/cpp-effects.h"

namespace eff = cpp_effects;

struct Put : eff::command<> {
    int64_t newState;
};
struct Get : eff::command<int64_t> {};

void put(int64_t s) { eff::invoke_command(Put{{}, s}); }
int64_t get() { return eff::invoke_command(Get{}); }

int stateful(void) {
    for (int i = 0; i < 10000000; i++) {
        put(get() + 1);
    }
    return 0;
}

int64_t value;

template <typename T> // the type of the handled computation
class State : public eff::handler<T, T, eff::no_manage<Put>, eff::no_manage<Get>> {
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

int main(void) {
    value = 0;
    eff::handle<State<int>>(stateful);
    printf("Final value is %ld\n", value);
}
