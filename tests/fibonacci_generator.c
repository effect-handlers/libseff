#include "seff.h"
#include "seff_types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void *fibonacci_generator(seff_coroutine_t *self, void *arg) {
    // The upper limit is passed to the coroutine as a void*
    int64_t limit = (int64_t)arg;
    int64_t a = 1, b = 0;
    for (size_t i = 0; i < limit; i++) {
        seff_yield(self, (void *)a);
        int64_t tmp = a;
        a = a + b;
        b = tmp;
    }
    return NULL;
}

int main(void) {
    seff_coroutine_t *fib = seff_coroutine_new(fibonacci_generator, (void *)10);
    seff_resumption_t res = seff_coroutine_start(fib);
    while (true) {
        // The extra argument to seff_resume will be passed to
        // fibonacci_generator as the return value of the call to seff_yield. In
        // this case, it is just ignored.
        int64_t next_fibonacci = (int64_t)seff_resume(res, NULL);
        res.sequence += 1;
        if (fib->state == FINISHED)
            break;
        printf("%ld\n", next_fibonacci);
    }
}
