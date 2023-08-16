#include <stdint.h>
#include <stdio.h>

int64_t value;

void __attribute__((noinline)) put(int64_t number) { value = number; }
int64_t __attribute__((noinline)) get(void) { return value; }

void stateful(void) {
    for (int i = 0; i < 10000000; i++) {
        put(get() + 1);
    }
}

int main(int argc, char **argv) {
    value = 0;

    stateful();

    printf("Final value is %ld\n", value);
}
