#include "libseff_effect_common.h"
#include "seff.h"
#include <stdio.h>

long SegsEffectMultiLookup(
    int v[], size_t v_size, int lookups[], size_t lookups_size, int streams) {

    size_t found_count = 0;
    size_t not_found_count = 0;

    int limit = streams;
    coro_queue q;
    coro_queue_init(&q);
    const size_t coro_size = 4096;

    seg_args *args = calloc(lookups_size, sizeof(seg_args));

    for (size_t i = 0; i < lookups_size; ++i) {
        args[i].first = v;
        args[i].len = v_size;
        args[i].key = lookups[i];
    }

    for (size_t next_arg = 0; next_arg < lookups_size;) {
        while (next_arg < lookups_size && limit > 0) {
            limit--;
            seff_coroutine_t *coro = seff_coroutine_new_sized(
                SegEffectBinarySearch, (void *)(args + next_arg), coro_size);
            next_arg++;
            // We assume SegBinarySearch will copy the values right after the first resume
            //   seff_resume(coro, NULL);
            coro_queue_enqueue(&q, (task_t){coro, NULL});
        }

        while (limit == 0) {
            task_t task = coro_queue_dequeue(&q);
            seff_coroutine_t *coro = task.coro;
            int64_t *toRead = task.extra;
            seff_eff_t *request = seff_handle(
                coro, toRead ? (void *)*toRead : NULL, HANDLES(deref));

            if (coro->state == FINISHED) {
                bool res = (bool) request ? 1 : 0;
                found_count += res;
                not_found_count += 1 - res;
                seff_coroutine_delete(coro);
                limit++;
            } else {
                switch (request->id) {
                    CASE_EFFECT(request, deref, {
                        prefetch_c((const char *)payload.addr);
                        coro_queue_enqueue(&q, (task_t){coro, (void *)payload.addr});
                        break;
                    });
                }
            }

        }
    }

    // no more to add, finish up
    while (limit < streams) {
        task_t task = coro_queue_dequeue(&q);
        seff_coroutine_t *coro = task.coro;
        int64_t *toRead = task.extra;
        seff_eff_t *request =
            seff_handle(coro, toRead ? (void *)*toRead : NULL, HANDLES(deref));

        if (coro->state == FINISHED) {
            bool res = (bool) request ? 1 : 0;
            found_count += res;
            not_found_count += 1 - res;
            seff_coroutine_delete(coro);
            limit++;
        } else {
            switch (request->id) {
                CASE_EFFECT(request, deref, {
                    prefetch_c((const char *)payload.addr);
                    coro_queue_enqueue(&q, (task_t){coro, (void *)payload.addr});
                    break;
                });
            }
        }
    }

    if (found_count + not_found_count != lookups_size)
        printf("BUG: found %zu, not-found: %zu total %zu, expected: %zu\n", found_count,
            not_found_count, found_count + not_found_count, lookups_size);

    return found_count;
}

long testSegsEffMulti(int v[], size_t v_size, int lookups[], size_t lookups_size, int streams) {
    return SegsEffectMultiLookup(v, v_size, lookups, lookups_size, streams);
}

int runner_c(long (*testFn)(int[], size_t, int[], size_t, int), int streams, const char *algo_name);

int main(int argc, const char **argv) {
    int streams = 7;

    if (argc == 2) {
        streams = atoi(argv[1]);
    }

    // 8 seems to be optimal
    return runner_c(testSegsEffMulti, streams, "seff");
}
