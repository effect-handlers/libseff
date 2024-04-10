// Based on https://github.com/koka-lang/libhandler/blob/master/test/test-excn.c

#include "libhandler.h"
#include <stdlib.h>

LH_DEFINE_EFFECT1(exception, runtime_error)
LH_DEFINE_VOIDOP1(exception, runtime_error, lh_string)

#ifndef HANDLER_KIND
#define HANDLER_KIND LH_OP_NORESUME
#endif

lh_value computation(lh_value arg) {
    exception_runtime_error("error");
    return lh_value_null;
}

static lh_value _excn_raise(lh_resume sc, lh_value local, lh_value arg) { return lh_value_int(1); }

static const lh_operation _excn_ops[] = {
    {HANDLER_KIND, LH_OPTAG(exception, runtime_error), &_excn_raise},
    {LH_OP_NULL, lh_op_null, NULL}};
static const lh_handlerdef excn_def = {LH_EFFECT(exception), NULL, NULL, NULL, _excn_ops};

lh_value excn_handle(lh_value (*action)(lh_value), lh_value arg) {
    return lh_handle(&excn_def, lh_value_null, action, arg);
}

int main(void) {
    size_t caught = 0;
    for (size_t i = 0; i < 1000000; i++) {
        caught += lh_int_value(excn_handle(computation, lh_value_long(42)));
    }
    printf("Caught %lu exceptions\n", caught);
    return 0;
}
