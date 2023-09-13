all: output/libseff output/libseff_async output/libseff_async_tk output/libseff_scheff output/libseff_scheff_suspend

include ../common.mk

output/libseff: libseff.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff.c -o $@ $(LDFLAGS_LIBSEFF)

output/libseff_async: libseff_async.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff_async.c -o $@ $(LDFLAGS_LIBSEFF)

output/libseff_async_tk: libseff_async_tk.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff_async_tk.c -o $@ $(LDFLAGS_LIBSEFF)

output/libseff_scheff: libseff_scheff.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff_scheff.c -o $@ $(LDFLAGS_LIBSEFF)

output/libseff_scheff_suspend: libseff_scheff_suspend.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff_scheff_suspend.c -o $@ $(LDFLAGS_LIBSEFF)
