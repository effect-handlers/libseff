#include "seff.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char fat_stack(void) {
    volatile char padding[10000];
    return padding[9999];
}

void *fn(void *arg) {
    seff_yield(seff_current_coroutine(), 0, NULL);
    fat_stack();
    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(fn, NULL);
    seff_resume_handling_all(k, NULL);
    seff_resume_handling_all(k, NULL);
}
