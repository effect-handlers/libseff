#include <stddef.h>
#include <stdio.h>

#include "co_routine.h"

typedef struct {
    bool end;
    bool put;
    int64_t state;
} state_payload_t;

void *stateful(void *arg) {
    state_payload_t *payload = (state_payload_t *)arg;
    for (int i = 0; i < 10000000; i++) {
        payload->put = false;
        co_yield_ct();
        int64_t next = payload->state + 1;

        payload->put = true;
        payload->state = next;
        co_yield_ct();
    }
    payload->end = true;
    return NULL;
}

int main() {

    stCoRoutine_t *k1;
    state_payload_t payload;
    payload.end = false;
    co_create(&k1, NULL, stateful, &payload);

    int value = 0;
    while (!payload.end) {
        co_resume(k1);
        if (payload.put) {
            value = payload.state;
        } else {
            payload.state = value;
        }
    }

    printf("Final value is %ld\n", value);

    return 0;
}
