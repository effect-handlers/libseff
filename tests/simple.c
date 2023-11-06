#include "seff.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *fn(void *arg) {
    seff_yield(seff_current_coroutine(), 0, NULL);
    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(fn, NULL);
    seff_resume(k, NULL);
    seff_resume(k, NULL);
}
