#include "libhandler.h"
#include <stdlib.h>

LH_DEFINE_EFFECT2(state, get, put)
LH_DEFINE_OP0(state, get, long)
LH_DEFINE_VOIDOP1(state, put, long)

#ifndef HANDLER_KIND
#define HANDLER_KIND LH_OP_GENERAL
#endif

lh_value stateful(lh_value arg) {
    for (int i = 0; i < 10000000; i++) {
        state_put(state_get() + 1);
    }
    return lh_value_null;
}

lh_value _state_result(lh_value local, lh_value arg) { return arg; }

long value;

lh_value _state_get(lh_resume rc, lh_value local, lh_value arg) {
    return lh_tail_resume(rc, value, value);
}

lh_value _state_put(lh_resume rc, lh_value local, lh_value arg) {
    value = arg;
    return lh_tail_resume(rc, arg, lh_value_null);
}

const lh_operation _state_ops[] = {{HANDLER_KIND, LH_OPTAG(state, get), &_state_get},
    {HANDLER_KIND, LH_OPTAG(state, put), &_state_put}, {LH_OP_NULL, lh_op_null, NULL}};
const lh_handlerdef state_def = {LH_EFFECT(state), NULL, NULL, &_state_result, _state_ops};

lh_value state_handle(lh_value (*action)(lh_value), int state0, lh_value arg) {
    return lh_handle(&state_def, lh_value_int(state0), action, arg);
}

int main(void) {
    value = 0;
    state_handle(stateful, 2, lh_value_null);
    printf("Final value is %ld\n", value);
}
