all: output/cppcoro output/libseff output/libco

include ../common.mk

output/libseff: libseff.cpp $(LIBSEFF_LIB) | output
	$(CXX) $(CXXFLAGS_LIBSEFF_SEGMENTED) libseff.cpp -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libco: libco.cpp $(LIBCO_LIB) | output
	$(CXX) $(CXXFLAGS_LIBCO) libco.cpp -o $@ $(LDFLAGS_LIBCO)

output/cppcoro: cppcoro.cpp $(CPPCORO_LIB) | output
	$(CXX) $(CXXFLAGS_CPPCORO) cppcoro.cpp -o $@ $(LDFLAGS_CPPCORO)

.PHONY: bench

FORCE:

output/bench-%.json: output/% FORCE | output
	hyperfine -L "stack size (bytes)" 0,1000,2000,3000,4000,5000 -w5  --export-json $@ -n "$*" "$< 10000000 0 {stack size (bytes)}"

bench: output/bench-libseff.json output/bench-libco.json output/bench-cppcoro.json