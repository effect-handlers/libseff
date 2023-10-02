#include "seff.h"
#include <stdio.h>

size_t context_switches = 0;

void *worker(seff_coroutine_t *self, void *_worker_id) {
    int64_t worker_id = (int64_t)_worker_id;

    int64_t result = 0;
    for (size_t i = 0; i < worker_id; i++) {
        result += i * i;
        context_switches += 1;
        seff_yield(self, NULL);
    }
    // printf("Coroutine %ld returning %ld\n", worker_id, result);
    context_switches += 1;
    return (void *)result;
}

#define MAX_COROUTINES 10000

typedef struct {
    seff_resumption_t coroutines[MAX_COROUTINES];
    int64_t next;
    int64_t size;
} coroutine_queue;

void enqueue(coroutine_queue *q, seff_resumption_t r) {
    q->coroutines[(q->next + q->size) % MAX_COROUTINES] = r;
    q->size += 1;
}

seff_resumption_t dequeue(coroutine_queue *q) {
    q->size -= 1;
    seff_resumption_t result = q->coroutines[q->next];
    q->next = (q->next + 1) % MAX_COROUTINES;
    return result;
}

int main(void) {
    coroutine_queue queue;
    queue.next = 0;
    queue.size = 0;

    for (int64_t i = 0; i < MAX_COROUTINES; i++) {
        enqueue(&queue, seff_coroutine_start(seff_coroutine_new(worker, (void *)i)));
    }

    int64_t results[MAX_COROUTINES];
    size_t done = 0;
    while (queue.size > 0) {
        // printf("Scheduling coroutine...\n");
        seff_resumption_t next = dequeue(&queue);
        context_switches += 1;
        void *result = seff_resume(next, NULL);
        if (next.coroutine->state == FINISHED) {
            results[done] = (int64_t)result;
            done++;
            seff_coroutine_delete(next.coroutine);
        } else {
            next.sequence += 1;
            enqueue(&queue, next);
        }
    }

    printf("All coroutines done\n");
    /*
    printf("Results: [");
    for (size_t i = 0; i < MAX_COROUTINES; i++) {
        printf("%ld%s", results[i], i < MAX_COROUTINES-1? ", ": "]\n");
    }
    */
    printf("%lu context switches\n", context_switches);
}
