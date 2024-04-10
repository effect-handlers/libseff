#include "mpeff.h"
#include <stdio.h>
#include <stdlib.h>

MPE_DEFINE_EFFECT1(exception, runtime_error)
MPE_DEFINE_VOIDOP1(exception, runtime_error, mpe_string_t)

#ifndef HANDLER_KIND
#define HANDLER_KIND MPE_OP_MULTI
#endif

mpe_voidp_t computation(mpe_voidp_t arg) {
    exception_runtime_error("error");
    return mpe_voidp_null;
}

static mpe_voidp_t _excn_raise(mpe_resume_t *sc, mpe_voidp_t local, mpe_voidp_t arg) {
    return mpe_voidp_int(1);
}

static const mpe_handlerdef_t excn_def = {MPE_EFFECT(exception), NULL,
    {{HANDLER_KIND, MPE_OPTAG(exception, runtime_error), &_excn_raise},
        {MPE_OP_NULL, mpe_op_null, NULL}}};

mpe_voidp_t excn_handle(mpe_voidp_t (*action)(mpe_voidp_t), mpe_voidp_t arg) {
    return mpe_handle(&excn_def, mpe_voidp_null, action, arg);
}

int main(void) {
    size_t caught = 0;
    for (size_t i = 0; i < 1000000; i++) {
        caught += mpe_int_voidp(excn_handle(computation, mpe_voidp_long(42)));
    }
    printf("Caught %lu exceptions\n", caught);
    return 0;
}
