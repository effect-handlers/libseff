#include "seff.h"
#include "tl_queue.h"
#include <stdio.h>

DEFINE_EFFECT(yield, 0, void, {});
DEFINE_EFFECT(fork, 1, void, {
    void *(*fn)(void *);
    void *arg;
});

void *worker(void *param) {
    int64_t id = (int64_t)param;
    for (int64_t iteration = 0; iteration < 10; iteration++) {
        printf("Worker %ld, iteration %ld\n", id, iteration);
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
                seff_coroutine_t *new = seff_coroutine_new(payload.fn, payload.arg);
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

int main(void) {
    with_scheduler(seff_coroutine_new(root, (void*)0));
    return 0;
}
