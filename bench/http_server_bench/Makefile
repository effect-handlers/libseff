all: output/libseff_simple output/libseff_stealing output/libseff_spilling output/libseff_epoll output/libscheff

include ../common.mk

output/%: %.c libseff_generic.c $(LIBSEFF_LIB) $(PICOHTTP_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) $(PICOHTTP_INCLUDE_DIRS) $< $(PICOHTTP_LIB) -o $@ $(LDFLAGS_LIBSEFF)
