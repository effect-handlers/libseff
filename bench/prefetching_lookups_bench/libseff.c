#include "libseff_infra.h"
#include "seff.h"
#include <stdio.h>

void *SeffBinarySearch(void *_arg) {
    seff_coroutine_t *self = seff_current_coroutine();
    search_args *arg = (search_args *)_arg;
    int const *first = arg->first;
    size_t len = arg->len;
    int val = arg->key;

    while (len > 0) {
        size_t half = len / 2;
        int const *middle = first + half;

        prefetch_c((const char *)middle);
        seff_yield(self, 0, (void *)-1);
        int x = *middle;

        if (x < val) {
            first = middle;
            ++first;
            len = len - half - 1;
        } else
            len = half;
        if (x == val) {
            return (void *)1;
        }
    }
    return (void *)0;
}

long SeffMultiLookup(int v[], size_t v_size, int lookups[], size_t lookups_size, int streams) {

    size_t found_count = 0;
    size_t not_found_count = 0;

    int limit = streams;
    coro_queue q;
    coro_queue_init(&q);
    const size_t coro_size = 256;

    search_args *args = calloc(lookups_size, sizeof(search_args));

    for (size_t i = 0; i < lookups_size; ++i) {
        args[i].first = v;
        args[i].len = v_size;
        args[i].key = lookups[i];
    }

    for (size_t next_arg = 0; next_arg < lookups_size;) {
        while (next_arg < lookups_size && limit > 0) {
            limit--;
            seff_coroutine_t *coro =
                seff_coroutine_new_sized(SeffBinarySearch, (void *)(args + next_arg), coro_size);
            next_arg++;
            coro_queue_enqueue(&q, (task_t){coro, NULL});
        }

        while (1) {
            seff_coroutine_t *coro = coro_queue_dequeue(&q).coro;
            int64_t res = (int64_t)seff_resume_handling_all(coro, NULL).payload;
            if (res == -1) {
                // keep looking next iter
                coro_queue_enqueue(&q, (task_t){coro, NULL});
            } else {
                // coroutine done
                found_count += res;
                not_found_count += 1 - res;
                // optimize
                seff_coroutine_delete(coro);
                limit++;
                break;
            }
        }
    }

    // no more to add, finish up
    while (limit < streams) {
        seff_coroutine_t *coro = coro_queue_dequeue(&q).coro;
        int64_t res = (int64_t)seff_resume_handling_all(coro, NULL).payload;
        if (res == -1) {
            // keep looking next iter
            coro_queue_enqueue(&q, (task_t){coro, NULL});
        } else {
            // coroutine done
            found_count += res;
            not_found_count += 1 - res;
            // optimize
            seff_coroutine_delete(coro);
            limit++;
        }
    }

    free(args);

    if (found_count + not_found_count != lookups_size)
        printf("BUG: found %zu, not-found: %zu total %zu, expected: %zu\n", found_count,
            not_found_count, found_count + not_found_count, lookups_size);

    return found_count;
}

int runner_c(long (*testFn)(int[], size_t, int[], size_t, int), int streams, const char *algo_name);

long testSeff(int v[], size_t v_size, int lookups[], size_t lookups_size, int streams) {
    return SeffMultiLookup(v, v_size, lookups, lookups_size, streams);
}

int main(int argc, const char **argv) {
    int streams = 8;

    if (argc == 2) {
        streams = atoi(argv[1]);
    }

    // 8 seems to be optimal
    return runner_c(testSeff, streams, "seff");
}
