#include "seff.h"
#include "seff_types.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *fn(seff_coroutine_t *self, void *arg) {
    seff_yield(self, NULL);
    return NULL;
}

void *myThreadFun(void *vargp) {
    seff_coroutine_t *k = seff_coroutine_new(fn, NULL);
    seff_resumption_t res = seff_coroutine_start(k);
    seff_resume(res, NULL);
    res.sequence += 1;
    seff_resume(res, NULL);
    return NULL;
}

#define THREADS 20000
#define REPS 1000
int main(void) {
    pthread_t thread_id[THREADS];
    for (int i = 0; i < REPS; i++) {
        printf("Before %d Threads, Rep %d\n", THREADS, i);
        for (int i = 0; i < THREADS; i++) {
            int err = pthread_create(&thread_id[i], NULL, myThreadFun, NULL);
            if (err != 0) {
                printf("Error at thread creation %d\n", i);
                exit(-1);
                break;
            }
        }
        for (int i = 0; i < THREADS; i++) {
            pthread_join(thread_id[i], NULL);
        }
        printf("After Threads\n");
    }
    return 0;
}
