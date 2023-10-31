#include "seff.h"
#include <stdio.h>

uint64_t counter;

void heavy_incr(void) {
    char a[8000];
    a[7998] = 'a';
    counter++;
}

void *coroutine(void *arg) {
    int reps = 100000000;
    while (reps--) {
        heavy_incr();
    }
    return NULL;
}

int main(void) {
    counter = 0;
    seff_coroutine_t *k = seff_coroutine_new(coroutine, NULL);
    puts("Created coroutine");
    seff_resume(k, NULL);
    seff_coroutine_delete(k);
    printf("Total counter: %lu", counter);
}
