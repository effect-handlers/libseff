all:  output/libseff_fixed output/libseff_baseline output/libseff_hot_split output/libseff_red_zone
dumps: output/libseff_fixed.dmp output/libseff_baseline.dmp output/libseff_hot_split.dmp output/libseff_red_zone.dmp

.PHONY: bench

FORCE:

include ../common.mk

output/libseff_fixed: libseff_fixed.c $(LIBSEFF_FIXED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_FIXED) libseff_fixed.c -o $@ $(LDFLAGS_LIBSEFF_FIXED)

output/libseff_baseline: libseff_baseline.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_baseline.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_hot_split: libseff_hot_split.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_hot_split.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_red_zone: libseff_red_zone.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_red_zone.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/%.dmp: output/% FORCE | output
	objdump -S --disassemble=bottom $< > $@


output/bench-libseff.json: all FORCE | output
	hyperfine -L "variation" _fixed,_baseline,_hot_split,_red_zone -w5  --export-json $@ -n "libseff" "output/libseff{variation}"

bench: output/bench-libseff.json