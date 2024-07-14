#include "libseff_effect_common.h"
#include "seff.h"
#include <stdio.h>

long SeffEffectMultiLookup(
    int v[], size_t v_size, int lookups[], size_t lookups_size, int streams) {

    size_t found_count = 0;
    size_t not_found_count = 0;

    int limit = streams;
    coro_queue q;
    coro_queue_init(&q);
    const size_t coro_size = 4096;

    search_args *args = calloc(lookups_size, sizeof(search_args));

    for (size_t i = 0; i < lookups_size; ++i) {
        args[i].first = v;
        args[i].len = v_size;
        args[i].key = lookups[i];
    }

    for (size_t next_arg = 0; next_arg < lookups_size;) {
        while (next_arg < lookups_size && limit > 0) {
            limit--;
            seff_coroutine_t *coro = seff_coroutine_new_sized(
                SeffEffectBinarySearch, (void *)(args + next_arg), coro_size);
            next_arg++;
            // We assume SeffBinarySearch will copy the values right after the first resume
            //   seff_resume_handling_all(coro, NULL);
            coro_queue_enqueue(&q, (task_t){coro, NULL});
        }

        while (limit == 0) {
            task_t task = coro_queue_dequeue(&q);
            seff_coroutine_t *coro = task.coro;
            int64_t *toRead = task.extra;
            seff_request_t request =
                seff_resume(coro, toRead ? (void *)*toRead : NULL, HANDLES(deref));
            switch (request.effect) {
                CASE_RETURN(request, {
                    bool res = (bool)payload.result ? 1 : 0;
                    found_count += res;
                    not_found_count += 1 - res;
                    seff_coroutine_delete(coro);
                    limit++;
                    break;
                });
                CASE_EFFECT(request, deref, {
                    prefetch_c((const char *)payload.addr);
                    coro_queue_enqueue(&q, (task_t){coro, (void *)payload.addr});
                    break;
                });
            }
        }
    }

    // no more to add, finish up
    while (limit < streams) {
        task_t task = coro_queue_dequeue(&q);
        seff_coroutine_t *coro = task.coro;
        int64_t *toRead = task.extra;
        seff_request_t request = seff_resume(coro, toRead ? (void *)*toRead : NULL, HANDLES(deref));

        switch (request.effect) {
            CASE_RETURN(request, {
                bool res = (bool)payload.result ? 1 : 0;
                found_count += res;
                not_found_count += 1 - res;
                seff_coroutine_delete(coro);
                limit++;
                break;
            });
            CASE_EFFECT(request, deref, {
                prefetch_c((const char *)payload.addr);
                coro_queue_enqueue(&q, (task_t){coro, (void *)payload.addr});
                break;
            });
        }
    }

    if (found_count + not_found_count != lookups_size)
        printf("BUG: found %zu, not-found: %zu total %zu, expected: %zu\n", found_count,
            not_found_count, found_count + not_found_count, lookups_size);

    return found_count;
}

long testSeffEffMulti(int v[], size_t v_size, int lookups[], size_t lookups_size, int streams) {
    return SeffEffectMultiLookup(v, v_size, lookups, lookups_size, streams);
}

int runner_c(long (*testFn)(int[], size_t, int[], size_t, int), int streams, const char *algo_name);

int main(int argc, const char **argv) {
    int streams = 7;

    if (argc == 2) {
        streams = atoi(argv[1]);
    }

    // 8 seems to be optimal
    return runner_c(testSeffEffMulti, streams, "seff");
}
