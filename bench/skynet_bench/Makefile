all: output/libseff output/libseff_async output/libseff_async_tk

include ../common.mk

output/libseff: libseff.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff.c -o $@ $(LDFLAGS_LIBSEFF)

output/libseff_async: libseff_async.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff_async.c -o $@ $(LDFLAGS_LIBSEFF)

output/libseff_async_tk: libseff_async_tk.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff_async_tk.c -o $@ $(LDFLAGS_LIBSEFF)
