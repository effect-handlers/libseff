all: output/naive output/no_alloc output/indirect_prop_t output/ptr_dbl_cast

include ../common.mk

output/%: %.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) $< -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)
