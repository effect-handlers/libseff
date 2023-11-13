#include <stdio.h>
#include "seff.h"

DEFINE_EFFECT(defer, 0, void, {
    void (*defer_fn)(void *);
    void *defer_arg;
});

void my_free(void *ptr) {
    printf("Freeing pointer %p\n", ptr);
    free(ptr);
}

void *allocate(size_t size) {
    void *ptr = malloc(size);
    printf("Allocated %lu bytes at pointer %p\n", size, ptr);
    PERFORM(defer, my_free, ptr);
    return ptr;
}

void my_fclose(FILE *file) {
    printf("Closing file %d\n", fileno(file));
    fclose(file);
}

FILE *temp_file(void) {
    FILE *file = tmpfile();
    printf("Opened temporary file with fd %d\n", fileno(file));
    PERFORM(defer, (void (*)(void*))my_fclose, file);
    return file;
}

void *code(void *arg) {
    void *ptr1 = allocate(256);
    void *ptr2 = allocate(512);
    FILE *fd = temp_file();

    fwrite(ptr1, sizeof(char), 256, fd);
    fwrite(ptr2, sizeof(char), 512, fd);

    return NULL;
}

void *handle_defer(seff_coroutine_t *k) {
    seff_request_t req = seff_handle(k, NULL, HANDLES(defer));
    switch (req.effect) {
        CASE_EFFECT(req, defer, {
            void *result = handle_defer(k);
            payload.defer_fn(payload.defer_arg);
            return result;
        });
        CASE_RETURN(req, { return payload.result; });
        default: exit(-1);
    }
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(code, NULL);
    handle_defer(k);
    seff_coroutine_delete(k);

    return 0;
}
