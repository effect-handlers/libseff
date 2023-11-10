include ../common.mk

all_libseff		:= output/libseff output/libseff_direct
all_native		:= output/c
all_cpp-effects	:= output/cpp-effects output/cpp-effects_optimal
all_libhandler	:= output/libhandler_tail_noop output/libhandler_tail output/libhandler_scoped output/libhandler_general
all_libmpeff	:= output/libmpeff_tail_noop output/libmpeff_tail output/libmpeff_scoped_once output/libmpeff_scoped output/libmpeff_once output/libmpeff_multi

all: $(all_libseff) $(all_native) $(all_cpp-effects) $(all_libhandler) $(all_libmpeff)

output/libseff: libseff.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_direct: libseff_direct.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_direct.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/c: c.c | output
	$(CC) $(CFLAGS) c.c -o $@ $(LDFLAGS)

output/cpp-effects: cpp-effects.cpp | output
	$(CXX) $(CXXFLAGS_CPP-EFFECTS) cpp-effects.cpp -o $@ $(LDFLAGS_CPP-EFFECTS)

output/cpp-effects_optimal: cpp-effects_optimal.cpp | output
	$(CXX) $(CXXFLAGS_CPP-EFFECTS) cpp-effects_optimal.cpp -o $@ $(LDFLAGS_CPP-EFFECTS)

# LH_OP_TAIL_NOOP
output/libhandler_tail_noop: libhandler.c $(LIBHANDLER_LIB) | output
	$(CC) $(CFLAGS_LIBHANDLER) -DHANDLER_KIND=LH_OP_TAIL_NOOP libhandler.c -o $@ $(LDFLAGS_LIBHANDLER)

# LH_OP_TAIL
output/libhandler_tail: libhandler.c $(LIBHANDLER_LIB) | output
	$(CC) $(CFLAGS_LIBHANDLER) -DHANDLER_KIND=LH_OP_TAIL libhandler.c -o $@ $(LDFLAGS_LIBHANDLER)

# LH_OP_SCOPED
output/libhandler_scoped: libhandler.c $(LIBHANDLER_LIB) | output
	$(CC) $(CFLAGS_LIBHANDLER) -DHANDLER_KIND=LH_OP_SCOPED libhandler.c -o $@ $(LDFLAGS_LIBHANDLER)

# LH_OP_GENERAL
output/libhandler_general: libhandler.c $(LIBHANDLER_LIB) | output
	$(CC) $(CFLAGS_LIBHANDLER) -DHANDLER_KIND=LH_OP_GENERAL libhandler.c -o $@ $(LDFLAGS_LIBHANDLER)

# MPE_OP_TAIL_NOOP
output/libmpeff_tail_noop: libmpeff.c $(LIBMPEFF_LIB) | output
	$(CC) $(CFLAGS_LIBMPROMPT) -DHANDLER_KIND=MPE_OP_TAIL_NOOP libmpeff.c -o $@ $(LDFLAGS_LIBMPROMPT)

# MPE_OP_TAIL
output/libmpeff_tail: libmpeff.c $(LIBMPEFF_LIB) | output
	$(CC) $(CFLAGS_LIBMPROMPT) -DHANDLER_KIND=MPE_OP_TAIL libmpeff.c -o $@ $(LDFLAGS_LIBMPROMPT)

# MPE_OP_SCOPED_ONCE
output/libmpeff_scoped_once: libmpeff.c $(LIBMPEFF_LIB) | output
	$(CC) $(CFLAGS_LIBMPROMPT) -DHANDLER_KIND=MPE_OP_SCOPED_ONCE libmpeff.c -o $@ $(LDFLAGS_LIBMPROMPT)

# MPE_OP_SCOPED
output/libmpeff_scoped: libmpeff.c $(LIBMPEFF_LIB) | output
	$(CC) $(CFLAGS_LIBMPROMPT) -DHANDLER_KIND=MPE_OP_SCOPED libmpeff.c -o $@ $(LDFLAGS_LIBMPROMPT)

# MPE_OP_ONCE
output/libmpeff_once: libmpeff.c $(LIBMPEFF_LIB) | output
	$(CC) $(CFLAGS_LIBMPROMPT) -DHANDLER_KIND=MPE_OP_ONCE libmpeff.c -o $@ $(LDFLAGS_LIBMPROMPT)

# MPE_OP_MULTI
output/libmpeff_multi: libmpeff.c $(LIBMPEFF_LIB) | output
	$(CC) $(CFLAGS_LIBMPROMPT) -DHANDLER_KIND=MPE_OP_MULTI libmpeff.c -o $@ $(LDFLAGS_LIBMPROMPT)

.PHONY: bench

FORCE:

HYPERFINE = hyperfine
HYP_ARGS = -w1 -r2
HYP_PARAMS = -L "depth" 0,10,20,40,60,80,100

output/bench-libseff.json: $(all_libseff) | output
	$(HYPERFINE) $(HYP_ARGS) $(HYP_PARAMS) -L "variation" ,_direct --export-json $@ -n "libseff" "output/libseff{variation} 10000000 {depth}"

output/bench-native.json: $(all_native) | output
	$(HYPERFINE) $(HYP_ARGS) $(HYP_PARAMS) -L "variation" "" --export-json $@ -n "native" "output/c{variation} 10000000 {depth}"

output/bench-cpp-effects.json: $(all_cpp-effects) | output
	$(HYPERFINE) $(HYP_ARGS) $(HYP_PARAMS) -L "variation" ,_optimal --export-json $@ -n "cpp-effects" "output/cpp-effects{variation} 10000000 {depth}"


output/bench-libhandler.json: $(all_libhandler) | output
	$(HYPERFINE) $(HYP_ARGS) $(HYP_PARAMS) -L "variation" _tail_noop,_general --export-json $@ -n "libhandler" "output/libhandler{variation} 10000000 {depth}"

output/bench-libmpeff.json: $(all_libmpeff) | output
	$(HYPERFINE) $(HYP_ARGS) $(HYP_PARAMS) -L "variation" _tail_noop,_once --export-json $@ -n "libmpeff" "output/libmpeff{variation} 10000000 {depth}"


output/state_good.png: graph.py output/bench-libseff.json output/bench-native.json output/bench-cpp-effects.json output/bench-libhandler.json output/bench-libmpeff.json | output
	cd ../.. ; python3 -m bench.state_paper_bench.graph

output/state_bad.png: graph.py output/bench-libseff.json output/bench-native.json output/bench-cpp-effects.json output/bench-libhandler.json output/bench-libmpeff.json | output
	cd ../.. ; python3 -m bench.state_paper_bench.graph
