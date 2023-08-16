#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

struct runtime_error {
    const char *msg;
    runtime_error(const char *msg) : msg(msg) {}
};

const char *error_msg = "Error!";

void *computation(void) { throw runtime_error(error_msg); }

int main(void) {
    size_t caught = 0;
    for (size_t i = 0; i < 1000000; i++) {
        try {
            computation();
        } catch (runtime_error exn) {
            caught++;
        }
    }
    printf("Caught %lu exceptions\n", caught);
    return 0;
}
