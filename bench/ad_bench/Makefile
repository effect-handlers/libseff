all: output/naive output/no_alloc output/indirect_prop_t output/ptr_dbl_cast output/reverse_taylor_recip_benchmark

include ../common.mk

output/reverse_taylor_recip_benchmark:  | output
	cd ocaml; dune clean; dune build
	cp ocaml/_build/default/bin/reverse_taylor_recip_benchmark.exe $@

output/%: %.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) $< -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

HYPER_PARAMS = -L "iterations" 1000,10000,50000,75000,100000,150000,200000

output/bench-libseff.json: output/no_alloc | output
	hyperfine $(HYPER_PARAMS) -w5   --export-json $@ -n "libseff" "$< {iterations}"

output/bench-ocaml.json: output/reverse_taylor_recip_benchmark | output
	hyperfine $(HYPER_PARAMS) -w5   --export-json $@ -n "ocaml" "$< {iterations}"

output/ad.png: graph.py output/bench-libseff.json output/bench-ocaml.json  | output
	cd ../.. ; python3 -m bench.ad_bench.graph

clean:
	rm -rf output
	cd ocaml; dune clean

