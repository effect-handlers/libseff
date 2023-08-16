#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_DEPTH 100

struct runtime_error {
    const char *msg;
    runtime_error(const char *msg) : msg(msg) {}
};

const char *error_msg = "Error!";

void *computation(size_t depth) {
    if (depth == 0) {
        throw runtime_error(error_msg);
    } else {
        return computation(depth - 1);
    }
}

int main(void) {
    size_t caught = 0;

    for (size_t i = 0; i < 100000; i++) {
        try {
            computation(MAX_DEPTH - 1);
        } catch (runtime_error exn) {
            caught++;
        }
    }
    printf("Caught %lu exceptions\n", caught);
}
