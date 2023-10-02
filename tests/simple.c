#include "seff.h"
#include "seff_types.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *fn(seff_coroutine_t *self, void *arg) {
    seff_yield(self, NULL);
    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(fn, NULL);
    seff_resumption_t res = seff_coroutine_start(k);
    seff_resume(res, NULL);
    res.sequence += 1;
    seff_resume(res, NULL);
}
