
all: output/libseff output/libseff_opt

include ../common.mk

output/libseff: libseff.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff.c -o $@ $(LDFLAGS_LIBSEFF)

output/libseff_opt: libseff_opt.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff_opt.c -o $@ $(LDFLAGS_LIBSEFF)
