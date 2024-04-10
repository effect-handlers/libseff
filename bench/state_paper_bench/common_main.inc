#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <iterations> <depth>\n", argv[0]);
        return 1;
    }

    int iterations;
    sscanf(argv[1], "%d", &iterations);

    int depth;
    sscanf(argv[2], "%d", &depth);

    // int64_t run_benchmark(int iterations, int depth);
    int64_t result = run_benchmark(iterations, depth);
    printf("Final value is %ld\n", result);
}
