
#include "cppcoro/recursive_generator.hpp"
#include "cppcoro/sync_wait.hpp"
#include "cppcoro/task.hpp"
#include <stdio.h>

typedef struct {
    bool put;
    int64_t state;
} state_payload_t;

cppcoro::recursive_generator<state_payload_t *> stateful() {
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

int main() {

    cppcoro::recursive_generator<state_payload_t *> k1 = stateful();

    cppcoro::recursive_generator<state_payload_t *>::iterator k1_iter = k1.begin();
    int64_t value = 0;
    for (cppcoro::recursive_generator<state_payload_t *>::iterator k1_iter = k1.begin();
         k1_iter != k1.end(); ++k1_iter) {
        state_payload_t *request = *k1_iter;
        if (request->put) {
            value = request->state;
        } else {
            request->state = value;
        }
    }

    printf("Final value is %ld\n", value);

    return 0;
}
