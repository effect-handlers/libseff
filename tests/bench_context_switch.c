#include "seff.h"
#include "seff_types.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GET 0
#define PUT 1

int64_t get(void) { return (int64_t)seff_perform(GET, NULL); }
void put(int64_t arg) { seff_perform(PUT, (void *)arg); }

void incr(void) { put(get() + 1); }

void *coroutine(seff_coroutine_t *self, void *arg) {
    while (1)
        incr();
}

int main(void) {
    size_t requests = 0;

    seff_coroutine_t *k = seff_coroutine_new(coroutine, NULL);

    seff_eff_t *request = seff_resume(seff_coroutine_start(k), NULL);

    int64_t state = 0;
    while (requests < 100 * 1000 * 1000) { // 100.000.000
        switch (request->id) {
        case GET:
            request = seff_resume(request->resumption, (void *)state);
            break;
        case PUT:
            state = (int64_t)request->payload;
            request = seff_resume(request->resumption, NULL);
            break;
        default:
            assert(0);
        }
        requests++;
    }
    printf("Final state: %ld\n", state);
    printf("Handled %ld requests\n", requests);
    return 0;
}
