all: output/naive output/no_alloc output/indirect_prop_t output/ptr_dbl_cast

include ../common.mk

output/%: %.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) $< -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/bench-%.json: output/% | output
	hyperfine -L "iterations" 1000,10000,50000,100000 -w5  --export-json $@ -n "$*" "$< {iterations}"

output/ad.png: output/bench-naive.json output/bench-no_alloc.json output/bench-indirect_prop_t.json output/bench-ptr_dbl_cast.json graph.py | output
	cd ../.. ; python3 -m bench.ad_bench.graph

bench: output/bench-naive.json output/bench-no_alloc.json output/bench-indirect_prop_t.json output/bench-ptr_dbl_cast.json
