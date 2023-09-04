all: output/naive output/no_alloc output/indirect_prop_t output/ptr_dbl_cast 

include ../common.mk

output/%: %.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) $< -o $@ $(LDFLAGS_LIBSEFF)
