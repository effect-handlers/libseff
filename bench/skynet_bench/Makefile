all: output/libseff output/libseff_async output/libseff_async_tk output/libseff_scheff_poll output/libseff_scheff_sleep

include ../common.mk

output/libseff: libseff.c skynet_common.h $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_async: libseff_async.c skynet_common.h $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_async.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_async_tk: libseff_async_tk.c skynet_common.h $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_async_tk.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_scheff_poll: libseff_scheff_poll.c skynet_common.h $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_scheff_poll.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_scheff_sleep: libseff_scheff_sleep.c skynet_common.h $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_scheff_sleep.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/bench-%.json: output/% | output
	hyperfine -L "threads" 1,4,8,16,32 -L depth  -w5  --export-json $@ -n "$*" "$< 10000000 0 {stack size (bytes)}"

output/context_switch.png: output/bench-libseff.json output/bench-libco.json output/bench-cppcoro.json | output
	cd ../.. ; python3 -m bench.context_switch_bench.graph

bench: output/bench-libseff.json output/bench-libseff_async.json output/bench-libseff_async_tk.json output/bench-libseff_scheff_poll.json output/bench-libseff_scheff_sleep.json