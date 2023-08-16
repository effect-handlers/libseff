#include "seff.h"
#include <stdio.h>

void heavy_hello(void) {
    char a[8000];
    a[7998] = 'a';
    puts("Hello world! I am a coroutine!");
}

void *coroutine(seff_coroutine_t *k, void *arg) {
    heavy_hello();
    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(coroutine, NULL);
    puts("Created coroutine");
    seff_resume(k, NULL);
    seff_coroutine_delete(k);
}
