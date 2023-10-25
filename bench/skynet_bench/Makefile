all: output/libseff output/libseff_async output/libseff_async_tk output/libseff_scheff_poll output/libseff_scheff_sleep

include ../common.mk

output/libseff: libseff.c skynet_common.h $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_async: libseff_async.c skynet_common.h $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_async.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_async_tk: libseff_async_tk.c skynet_common.h $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_async_tk.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_scheff_poll: libseff_scheff_poll.c skynet_common.h $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_scheff_poll.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_scheff_sleep: libseff_scheff_sleep.c skynet_common.h $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_scheff_sleep.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)
