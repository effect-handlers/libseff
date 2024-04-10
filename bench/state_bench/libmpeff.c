#include "mpeff.h"
#include <stdio.h>
#include <stdlib.h>

MPE_DEFINE_EFFECT2(state, get, put)
MPE_DEFINE_OP0(state, get, long)
MPE_DEFINE_VOIDOP1(state, put, long)

#ifndef HANDLER_KIND
#define HANDLER_KIND MPE_OP_MULTI
#endif

mpe_voidp_t stateful(mpe_voidp_t arg) {
    for (int i = 0; i < 10000000; i++) {
        state_put(state_get() + 1);
    }
    return mpe_voidp_null;
}

int64_t value;

mpe_voidp_t _state_get(mpe_resume_t *rc, mpe_voidp_t local, mpe_voidp_t arg) {
    return mpe_resume_tail(rc, (void *)value, (void *)value);
}

mpe_voidp_t _state_put(mpe_resume_t *rc, mpe_voidp_t local, mpe_voidp_t arg) {
    value = (uintptr_t)arg;
    return mpe_resume_tail(rc, arg, mpe_voidp_null);
}

const mpe_handlerdef_t state_def = {MPE_EFFECT(state), NULL,
    {{HANDLER_KIND, MPE_OPTAG(state, get), &_state_get},
        {HANDLER_KIND, MPE_OPTAG(state, put), &_state_put}, {MPE_OP_NULL, mpe_op_null, NULL}}};

mpe_voidp_t state_handle(mpe_voidp_t (*action)(mpe_voidp_t), int64_t state0, mpe_voidp_t arg) {
    return mpe_handle(&state_def, (void *)state0, action, arg);
}

int main(void) {
    value = 0;
    state_handle(stateful, 2, mpe_voidp_null);
    printf("Final value is %ld\n", value);
}
