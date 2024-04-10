#include "libhandler.h"
#include <stdlib.h>

LH_DEFINE_EFFECT2(state, get, put)
LH_DEFINE_OP0(state, get, long)
LH_DEFINE_VOIDOP1(state, put, long)

LH_DEFINE_EFFECT1(exceptions, error)
LH_DEFINE_VOIDOP0(exceptions, error)

int64_t value;

lh_value _state_get(lh_resume rc, lh_value local, lh_value arg) {
    return lh_tail_resume(rc, value, value);
}

lh_value _state_put(lh_resume rc, lh_value local, lh_value arg) {
    value = arg;
    return lh_tail_resume(rc, arg, lh_value_null);
}

const lh_operation _state_ops[] = {{HANDLER_KIND, LH_OPTAG(state, get), &_state_get},
    {HANDLER_KIND, LH_OPTAG(state, put), &_state_put}, {LH_OP_NULL, lh_op_null, NULL}};

const lh_handlerdef state_def = {LH_EFFECT(state), NULL, NULL, NULL, _state_ops};

lh_value state_handle(lh_value (*action)(lh_value), int state0, lh_value arg) {
    return lh_handle(&state_def, lh_value_int(state0), action, arg);
}

lh_value _exceptions_error(lh_resume rc, lh_value local, lh_value arg) { return lh_value_null; }

const lh_operation _exceptions_ops[] = {
    {HANDLER_KIND, LH_OPTAG(exceptions, error), &_exceptions_error},
    {LH_OP_NULL, lh_op_null, NULL}};

const lh_handlerdef exceptions_def = {LH_EFFECT(exceptions), NULL, NULL, NULL, _exceptions_ops};

lh_value exceptions_handle(lh_value (*action)(lh_value), lh_value arg) {
    return lh_handle(&exceptions_def, lh_value_null, action, arg);
}

lh_value stateful(lh_value arg) {
    int depth = lh_int_value(arg);
    if (depth == 0) {
        for (int i = state_get(); i > 0; i = state_get()) {
            state_put(i - 1);
        }
    } else {
        exceptions_handle(stateful, lh_value_int(depth - 1));
    }
    return lh_value_null;
}

int64_t run_benchmark(int iterations, int depth) {
    value = iterations;
    state_handle(stateful, 2, lh_value_int(depth));
    return value;
}

#include "common_main.inc"
