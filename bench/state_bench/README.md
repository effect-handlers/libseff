The `state_bench` benchmark will use two effects, `get` that will get the
value from a local state variable, and `put` that will update it.
It will repeat this for `10000000` times, expecting the final value
to be `10000000`.


This is the example from `libseff`, the other ones are similar:

```c
void *stateful(seff_coroutine_t *self, void *_arg) {
    for (int i = 0; i < 10000000; i++) {
        put(self, get(self) + 1);
    }
    return NULL;
}
```

The goal of this benchamrk is to test context switches, since the benchmark
performs ~20000000 context switches with little logic inbetween.

## libhandler and libmpropmt

When comparing against these libraries, there's the question of whether we should:
1. Use them properly, and setting the handlers as tail resuming handlers without operations
   (which will actually not perform any kind of context switch); or
2. use them fairly, and set up the handlers as a more general kind, bringing them closer to the
   abilities available to libseff handlers (and actually performing context switches).

_Arguably, we should give `libseff` similar tail resumption handlers and find another effect to compare_