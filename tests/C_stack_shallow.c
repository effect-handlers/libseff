#include "../src/seff.h"
#include <stdio.h>

void *stacky(void *a) {
    int m = *(int *)a;
    m--;
    if (m) {
        void *res = stacky((void *)&m);
        return (void *)((int64_t)res + 1);
    }
    return (void *)73330;
}

int main(void) {
    // Change these values around to enforce extra segments

    int metaReps = 60000;
    void *res;
    while (metaReps--) {
        int reps = 1; // 1000;

        while (reps < 100) {
            res = stacky((void *)&reps);
            reps++;
        }
    }

    printf("%ld", (int64_t)res);
}
