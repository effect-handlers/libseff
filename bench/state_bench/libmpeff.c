#include "mpeff.h"
#include <stdio.h>
#include <stdlib.h>

MPE_DEFINE_EFFECT2(state, get, put)
MPE_DEFINE_OP0(state, get, long)
MPE_DEFINE_VOIDOP1(state, put, long)

#ifndef HANDLER_KIND
#define HANDLER_KIND MPE_OP_MULTI
#endif

#define lh_value void *
#define lh_value_null NULL

lh_value stateful(lh_value arg) {
    for (int i = 0; i < 10000000; i++) {
        state_put(state_get() + 1);
    }
    return lh_value_null;
}

lh_value _state_result(lh_value local, lh_value arg) { return arg; }

int64_t value;

lh_value _state_get(mpe_resume_t *rc, lh_value local, lh_value arg) {
    return mpe_resume_tail(rc, (void *)value, (void *)value);
}

lh_value _state_put(mpe_resume_t *rc, lh_value local, lh_value arg) {
    value = (uintptr_t)arg;
    return mpe_resume_tail(rc, arg, lh_value_null);
}

const mpe_handlerdef_t state_def = {MPE_EFFECT(state), NULL,
    {{HANDLER_KIND, MPE_OPTAG(state, get), &_state_get},
        {HANDLER_KIND, MPE_OPTAG(state, put), &_state_put}, {MPE_OP_NULL, mpe_op_null, NULL}}};

lh_value state_handle(lh_value (*action)(lh_value), int64_t state0, lh_value arg) {
    return mpe_handle(&state_def, (void *)state0, action, arg);
}

int main(void) {
    value = 0;
    state_handle(stateful, 2, lh_value_null);
    printf("Final value is %ld\n", value);
}
