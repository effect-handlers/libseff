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

circular_array_t *circular_array_resize(circular_array_t *arr, int64_t bottom, int64_t top) {
    int64_t new_size = arr->size << 1;
    int64_t new_mask = new_size - 1;

    circular_array_t *new_array =
        malloc(sizeof(circular_array_t) + new_size * sizeof(_Atomic(void *)));

    new_array->size = new_size;
    new_array->mask = new_mask;

    _Atomic(void *) *buffer = arr->buffer;
    int64_t old_mask = arr->mask;
    for (int64_t i = top; i < bottom; i++) {
        void *old_value = atomic_load_explicit(&buffer[i & old_mask], memory_order_relaxed);
        atomic_store_explicit(&new_array->buffer[i & new_mask], old_value, memory_order_relaxed);
    }

    return new_array;
}
