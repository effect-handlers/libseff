#include <assert.h>
#include <stdatomic.h>
#include <stdlib.h>

typedef struct circular_array_t {
    int64_t size;
    int64_t mask;
    _Atomic(void *) buffer[];
} circular_array_t;

circular_array_t *circular_array_new(size_t log_size) {
    assert(log_size < 64);
    int64_t array_size = 1 << log_size;
    int64_t array_mask = array_size - 1;

    circular_array_t *new_array =
        malloc(sizeof(circular_array_t) + array_size * sizeof(_Atomic(void *)));
    new_array->size = array_size;
    new_array->mask = array_mask;
    return new_array;
}

#define CA_GET(arr, idx) atomic_load_explicit(&arr->buffer[idx & arr->mask], memory_order_relaxed)
#define CA_SET(arr, idx, elt) \
    atomic_store_explicit(&arr->buffer[idx & arr->mask], elt, memory_order_relaxed)

circular_array_t *circular_array_resize(circular_array_t *arr, int64_t bottom, int64_t top) {
    assert(__builtin_clz(arr->size) > 0);
    int64_t new_size = arr->size << 1;
    int64_t new_mask = new_size - 1;

    circular_array_t *new_array =
        malloc(sizeof(circular_array_t) + new_size * sizeof(_Atomic(void *)));

    new_array->size = new_size;
    new_array->mask = new_mask;

    for (int64_t i = top; i < bottom; i++) {
        CA_SET(new_array, i, CA_GET(arr, i));
    }

    return new_array;
}
