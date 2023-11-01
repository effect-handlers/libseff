#include "seff.h"
#include "tl_queue.h"
#include <stdio.h>
#include <string.h>

DEFINE_EFFECT(yield, 0, void, {});
DEFINE_EFFECT(fork, 1, void, {
    void *(*thread_fn)(void *);
    void *thread_arg;
});
DEFINE_EFFECT(print, 2, void, { char *msg; });

void *worker(void *param) {
    int64_t id = (int64_t)param;
    for (int64_t iteration = 0; iteration < 10; iteration++) {
        char msg[256];
        sprintf(msg, "Worker %ld, iteration %ld\n", id, iteration);
        PERFORM(print, msg);
        PERFORM(yield);
    }
    return NULL;
}

void *root(void *param) {
    for (int64_t i = 1; i <= 10; i++) {
        PERFORM(fork, worker, (void *)(i));
    }
    return NULL;
}

void with_scheduler(seff_coroutine_t *initial_coroutine) {
    effect_set handles_scheduler = HANDLES(yield) | HANDLES(fork);

    tl_queue_t queue;
    tl_queue_init(&queue, 5);

    tl_queue_push(&queue, initial_coroutine);

    while (!tl_queue_empty(&queue)) {
        seff_coroutine_t *next = (seff_coroutine_t *)tl_queue_steal(&queue);
        seff_request_t req = seff_handle(next, NULL, handles_scheduler);
        switch (req.effect) {
            CASE_EFFECT(req, yield, {
                tl_queue_push(&queue, (struct task_t *)next);
                break;
            });
            CASE_EFFECT(req, fork, {
                tl_queue_push(&queue, (struct task_t *)next);
                seff_coroutine_t *new = seff_coroutine_new(payload.thread_fn, payload.thread_arg);
                tl_queue_push(&queue, (struct task_t *)new);
                break;
            });
            CASE_RETURN(req, {
                seff_coroutine_delete(next);
                break;
            });
        }
    }
}

void *default_print(void *print_payload) {
    EFF_PAYLOAD_T(print) payload = *(EFF_PAYLOAD_T(print) *)(print_payload);
    fputs(payload.msg, stdout);
    return NULL;
}

void with_output_to_buffer(char *buffer, seff_coroutine_t *handled) {
    while (true) {
        seff_request_t req = seff_handle(handled, NULL, HANDLES(print));
        switch (req.effect) {
            CASE_EFFECT(req, print, {
                strcpy(buffer, payload.msg);
                buffer += strlen(payload.msg);
                break;
            });
            CASE_RETURN(req, { return; });
        }
    }
}

void *run_threads(void *arg) {
    with_scheduler(seff_coroutine_new(root, (void *)0));
    return 0;
}

int main(void) {
    seff_set_default_handler(EFF_ID(print), default_print);
    with_scheduler(seff_coroutine_new(root, (void *)0));

    char buffer[10 * 10 * 255];
    with_output_to_buffer(buffer, seff_coroutine_new(run_threads, NULL));
    printf("Total output: %lu bytes\n", strlen(buffer));

    return 0;
}
