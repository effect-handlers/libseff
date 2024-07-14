#include "seff.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void *fibonacci_generator(void *arg) {
    seff_coroutine_t *self = seff_current_coroutine();
    // The upper limit is passed to the coroutine as a void*
    int64_t limit = (int64_t)arg;
    int64_t a = 1, b = 0;
    for (size_t i = 0; i < limit; i++) {
        seff_yield(self, 0, (void *)a);
        int64_t tmp = a;
        a = a + b;
        b = tmp;
    }
    return NULL;
}

int main(void) {
    seff_coroutine_t *fib = seff_coroutine_new(fibonacci_generator, (void *)10);
    while (true) {
        // The extra argument to seff_resume_handling_all will be passed to
        // fibonacci_generator as the return value of the call to seff_yield. In
        // this case, it is just ignored.
        seff_request_t req = seff_resume_handling_all(fib, NULL);

        if (seff_finished(req))
            break;
        int64_t next_fibonacci = (int64_t)req.payload;
        printf("%ld\n", next_fibonacci);
    }
}
