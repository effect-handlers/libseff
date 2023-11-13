#include "seff.h"

DEFINE_EFFECT(eff1, 0, void, {});
DEFINE_EFFECT(eff2, 1, void, {});

void *g(void *arg) {
    PERFORM(eff1);
    PERFORM(eff2);
    return NULL;
}

void *f(void *arg) {
    seff_coroutine_t *k2 = seff_coroutine_new(g, NULL);    
    seff_handle(k2, NULL, HANDLES(eff2));
    seff_handle(k2, NULL, HANDLES(eff2));
    return NULL;
}
void main() {
    seff_coroutine_t *k1 = seff_coroutine_new(f, NULL);
    seff_handle(k1, NULL, HANDLES(eff1) | HANDLES(eff2));
    seff_handle(k1, NULL, HANDLES(eff1) | HANDLES(eff2));
}
