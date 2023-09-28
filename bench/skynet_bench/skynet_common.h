#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BRANCHING_FACTOR 10

void print_usage(char *self) {
    printf("Usage: %s [--depth M] [--threads N]\n", self);
    exit(-1);
}

typedef int64_t(skynet_t)(int n_workers, int depth);

int runner(int argc, char **argv, skynet_t bench, const char *bench_name) {
    int n_workers = 8;
    int depth = 7;

    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--threads") == 0) {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
            }
            int res = sscanf(argv[i + 1], "%d", &n_workers);
            if (res <= 0 || n_workers <= 0) {
                print_usage(argv[0]);
            }
            i++;
        } else if (strcmp(argv[i], "--depth") == 0) {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
            }
            int res = sscanf(argv[i + 1], "%d", &depth);
            if (res <= 0 || depth <= 0) {
                print_usage(argv[0]);
            }
            i++;
        } else {
            print_usage(argv[0]);
        }
    }

    printf(
        "Running skynet benchmark %s with %d workers and %d depth\n", bench_name, n_workers, depth);

    int64_t res = bench(n_workers, depth);

    printf("Total: %ld\n", res);

    return 0;
}
