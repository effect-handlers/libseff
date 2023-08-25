#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <mprompt.h>

typedef struct {
    bool put;
    int64_t state;
    mp_resume_t* res;
} state_payload_t;

static void* state_resumption(mp_resume_t* r, void* _arg) {
    state_payload_t* arg = (state_payload_t*)_arg;
    arg->res = r;
    return (void*)arg;
}

void put(mp_prompt_t* parent, int64_t number) {
    state_payload_t payload;
    payload.state = number;
    payload.put = true;
    mp_yield(parent, &state_resumption, (void*)&payload);
}

int64_t get(mp_prompt_t* parent) {
    state_payload_t payload;
    payload.put = false;
    return (int64_t)(uintptr_t)mp_yield(parent, &state_resumption, (void*)&payload);
}

void* stateful(mp_prompt_t *parent, void *_arg) {
    for (int i = 0; i < 10000000; i++) {
        put(parent, get(parent) + 1);
    }
    return NULL;
}

int main() {
    mp_config_t config = mp_config_default();
    // configure here
    mp_init(&config);

    int64_t value = 0;

    state_payload_t* request = (state_payload_t*)mp_prompt( &stateful, NULL );
    while (request != NULL) {
        if (request->put) {
            value = request->state;
            request = mp_resume(request->res, NULL);
        } else {
            request = mp_resume(request->res, (void*)value);
        }
    }

    printf("Final value is %ld\n", value);

    return 0;
}
