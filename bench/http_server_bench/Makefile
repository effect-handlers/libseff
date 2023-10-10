all: output/libscheff

include ../common.mk

output/%: %.c $(LIBSEFF_LIB) $(PICOHTTP_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) $(PICOHTTP_INCLUDE_DIRS) $< $(PICOHTTP_LIB) -o $@ $(LDFLAGS_LIBSEFF)
