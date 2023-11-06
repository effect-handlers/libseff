#include "libseff_infra.h"
#include "seff.h"

// In a templated world, this could be templated
DEFINE_EFFECT(deref, 5, int64_t, { int const *addr; });

void *SeffEffectBinarySearch(void *_arg) {
    search_args *arg = (search_args *)_arg;
    int const *first = arg->first;
    size_t len = arg->len;
    int val = arg->key;

    while (len > 0) {
        size_t half = len / 2;
        int const *middle = first + half;

        int x = PERFORM(deref, middle);

        if (x < val) {
            first = middle;
            ++first;
            len = len - half - 1;
        } else
            len = half;
        if (x == val) {
            return (void*) true;
        }
    }
    return (void*) false;
}
