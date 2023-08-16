#include "seff.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char fat_stack(void) {
    volatile char padding[10000];
    return padding[9999];
}

void *fn(seff_coroutine_t *self, void *arg) {
    seff_yield(self, NULL);
    fat_stack();
    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(fn, NULL);
    seff_resume(k, NULL);
    seff_resume(k, NULL);
}
