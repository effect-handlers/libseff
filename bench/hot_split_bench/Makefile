all:  output/libseff_fixed output/libseff_baseline output/libseff_hot_split output/libseff_red_zone output/native output/libseff_dealloc
dumps: output/libseff_fixed.dmp output/libseff_baseline.dmp output/libseff_hot_split.dmp output/libseff_red_zone.dmp output/native.dmp output/libseff_dealloc.dmp

.PHONY: bench

FORCE:

include ../common.mk

output/libseff_fixed: libseff_fixed.c common.inc $(LIBSEFF_FIXED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_FIXED) libseff_fixed.c -o $@ $(LDFLAGS_LIBSEFF_FIXED)

output/libseff_baseline: libseff_baseline.c common.inc $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_baseline.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_hot_split: libseff_hot_split.c common.inc $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_hot_split.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_dealloc: libseff_dealloc.c common.inc $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_dealloc.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_red_zone: libseff_red_zone.c common.inc $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_red_zone.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/native: native.c common.inc | output
	$(CC) $(CFLAGS) native.c -o $@ $(LDFLAGS)

# We care about the dump to see exactly the generated code
output/%.dmp: output/% | output
	objdump -S --disassemble=bottom $< > $@

output/bench-libseff.json: output/libseff_fixed output/libseff_baseline output/libseff_hot_split output/libseff_red_zone output/libseff_dealloc | output
	hyperfine -L "variation" _fixed,_baseline,_hot_split,_dealloc -w5  --export-json $@ -n "libseff" "output/libseff{variation}"

output/bench-native.json: output/native | output
	hyperfine -L "variation" "" -w5  --export-json $@ -n "native" "output/native{variation}"

output/hot_split.png: output/bench-libseff.json output/bench-native.json graph.py | output
	cd ../.. ; python3 -m bench.hot_split_bench.graph

bench: output/bench-libseff.json output/bench-native.json
