#include "mpeff.h"
#include <stdio.h>
#include <stdlib.h>

MPE_DEFINE_EFFECT2(state, get, put)
MPE_DEFINE_OP0(state, get, long)
MPE_DEFINE_VOIDOP1(state, put, long)

MPE_DEFINE_EFFECT1(exceptions, error)
MPE_DEFINE_VOIDOP0(exceptions, error)

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

mpe_voidp_t _exceptions_error(mpe_resume_t *rc, mpe_voidp_t local, mpe_voidp_t arg) {
    return mpe_voidp_null;
}

const mpe_handlerdef_t exceptions_def = {MPE_EFFECT(exceptions), NULL,
    {{HANDLER_KIND, MPE_OPTAG(exceptions, error), &_exceptions_error},
        {MPE_OP_NULL, mpe_op_null, NULL}}};

mpe_voidp_t exceptions_handle(mpe_voidp_t (*action)(mpe_voidp_t), mpe_voidp_t arg) {
    return mpe_handle(&exceptions_def, NULL, action, arg);
}

mpe_voidp_t stateful(mpe_voidp_t arg) {
    int depth = mpe_int_voidp(arg);
    if (depth == 0) {
        for (int i = state_get(); i > 0; i = state_get()) {
            state_put(i - 1);
        }
    } else {
        exceptions_handle(stateful, mpe_voidp_int(depth - 1));
    }
    return mpe_voidp_null;
}

int64_t run_benchmark(int iterations, int depth) {
    value = iterations;
    state_handle(stateful, 2, mpe_voidp_int(depth));
    return value;
}

#include "common_main.inc"