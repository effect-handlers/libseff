
#include "cppcoro/async_generator.hpp"
#include "cppcoro/sync_wait.hpp"
#include "cppcoro/task.hpp"
#include <stdio.h>

typedef struct {
    bool put;
    int64_t state;
} state_payload_t;

cppcoro::async_generator<state_payload_t *> stateful() {
    state_payload_t payload;
    for (int i = 0; i < 10000000; i++) {
        payload.put = false;
        co_yield &payload;
        int64_t next = payload.state + 1;
        payload.put = true;
        payload.state = next;
        co_yield &payload;
    }
    co_return;
}

cppcoro::task<int64_t> take(cppcoro::async_generator<state_payload_t *> &k1) {
    int64_t value = 0;
    for (cppcoro::async_generator<state_payload_t *>::iterator k1_iter = co_await k1.begin();
         k1_iter != k1.end(); co_await ++k1_iter) {
        state_payload_t *request = *k1_iter;
        if (request->put) {
            value = request->state;
        } else {
            request->state = value;
        }
    }

    co_return value;
}

int main() {

    cppcoro::async_generator<state_payload_t *> k1 = stateful();

    int64_t value = cppcoro::sync_wait(take(k1));

    printf("Final value is %ld\n", value);

    return 0;
}
