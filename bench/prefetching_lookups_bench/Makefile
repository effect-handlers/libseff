all: output/naive output/sm output/coro output/libseff output/libseff_effect_coro output/libseff_effect_naive

include ../common.mk

output/naive: external/naive.cpp external/runner.hpp external/rng.h | output
	$(CXX) $(CXXFLAGS) -fcoroutines-ts -std=c++2a -stdlib=libc++ -o $@ external/naive.cpp $(LDFLAGS)

output/sm: external/sm.cpp external/runner.hpp external/rng.h | output
	$(CXX) $(CXXFLAGS) -fcoroutines-ts -std=c++2a -stdlib=libc++ -o $@ external/sm.cpp $(LDFLAGS)

output/coro: external/coro.cpp external/coro_infra.h external/runner.hpp external/rng.h| output
	$(CXX) $(CXXFLAGS) -fcoroutines-ts -std=c++2a -stdlib=libc++ -o $@ external/coro.cpp $(LDFLAGS)

output/runner_c.o: external/runner_c.cpp external/runner.hpp  external/rng.h| output
	$(CXX) $(CXXFLAGS) -fcoroutines-ts -std=c++2a -stdlib=libc++ -o $@ -c external/runner_c.cpp

output/libseff: libseff.c output/runner_c.o $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) -o output/libseff.o -c libseff.c
	$(CXX) $(CXXFLAGS) -stdlib=libc++ -o $@ output/libseff.o output/runner_c.o $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_effect_coro: libseff_effect_coro.c libseff_effect_common.h output/runner_c.o $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) -o output/libseff_effect_coro.o -c libseff_effect_coro.c
	$(CXX) $(CXXFLAGS) -stdlib=libc++ -o $@ output/libseff_effect_coro.o output/runner_c.o $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_effect_naive: libseff_effect_naive.c libseff_effect_common.h output/runner_c.o $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) -o output/libseff_effect_naive.o -c libseff_effect_naive.c
	$(CXX) $(CXXFLAGS) -stdlib=libc++ -o $@ output/libseff_effect_naive.o output/runner_c.o $(LDFLAGS_LIBSEFF_SEGMENTED)


output/bench-%.json: output/% | output
	hyperfine -r10 -P streams 1 20 -w1  --export-json $@ -n "$*" "$< {streams}"

output/prefetch_all.png: output/bench-naive.json output/bench-sm.json output/bench-coro.json output/bench-libseff.json output/bench-libseff_effect_coro.json output/bench-libseff_effect_naive.json graph.py | output
	cd ../.. ; python3 -m bench.prefetching_lookups_bench.graph

output/prefetch_best.png: output/bench-naive.json output/bench-sm.json output/bench-coro.json output/bench-libseff.json output/bench-libseff_effect_coro.json output/bench-libseff_effect_naive.json graph.py | output
	cd ../.. ; python3 -m bench.prefetching_lookups_bench.graph

bench: output/bench-naive.json output/bench-sm.json output/bench-coro.json output/bench-libseff.json output/bench-libseff_effect_coro.json output/bench-libseff_effect_naive.json