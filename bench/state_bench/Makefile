include ../common.mk

all: $(all_libseff) $(all_native) $(all_cpp-effects) $(all_libhandler) $(all_libmpeff) $(all_libmprompt) $(all_cppcoro) $(all_libco)

all_libseff		:= output/libseff output/libseff_direct
all_native		:= output/c
all_cpp-effects	:= output/cpp-effects output/cpp-effects_plain
all_cppcoro		:= output/cppcoro_async_generator output/cppcoro_recursive_generator
all_libhandler	:= output/libhandler_tail_noop output/libhandler_tail  output/libhandler_general
# output/libhandler_scoped Removed since there appears to be a bug
all_libmpeff	:= output/libmpeff_tail_noop output/libmpeff_tail output/libmpeff_scoped_once output/libmpeff_scoped output/libmpeff_once output/libmpeff_multi
all_libmprompt	:= output/libmprompt
all_libco		:= output/libco


output/libseff: libseff.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_direct: libseff_direct.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_direct.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/c: c.c | output
	$(CC) $(CFLAGS) c.c -o $@ $(LDFLAGS)

output/cpp-effects: cpp-effects.cpp | output
	$(CXX) $(CXXFLAGS_CPP-EFFECTS) cpp-effects.cpp -o $@ $(LDFLAGS_CPP-EFFECTS)

output/cpp-effects_plain: cpp-effects_plain.cpp | output
	$(CXX) $(CXXFLAGS_CPP-EFFECTS) cpp-effects_plain.cpp -o $@ $(LDFLAGS_CPP-EFFECTS)

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

output/libmprompt: libmprompt.c $(LIBMPROMPT_LIB) | output
	$(CC) $(CFLAGS_LIBMPROMPT) libmprompt.c -o $@ $(LDFLAGS_LIBMPROMPT)

output/cppcoro_async_generator: cppcoro_async_generator.cpp $(CPPCORO_LIB) | output
	$(CXX) $(CXXFLAGS_CPPCORO) cppcoro_async_generator.cpp -o $@ $(LDFLAGS_CPPCORO)

output/cppcoro_recursive_generator: cppcoro_recursive_generator.cpp $(CPPCORO_LIB) | output
	$(CXX) $(CXXFLAGS_CPPCORO) cppcoro_recursive_generator.cpp -o $@ $(LDFLAGS_CPPCORO)

output/libco: libco.cpp $(LIBCO_LIB) | output
	$(CXX) $(CXXFLAGS_LIBCO) libco.cpp -o $@ $(LDFLAGS_LIBCO)

.PHONY: bench

FORCE:

output/bench-libseff.json: $(all_libseff) | output
	hyperfine -L "variation" ,_direct -w5  --export-json $@ -n "libseff" "output/libseff{variation}"

output/bench-native.json: $(all_native) | output
	hyperfine -L "variation" "" -w5  --export-json $@ -n "native" "output/c{variation}"

output/bench-cpp-effects.json: $(all_cpp-effects) | output
	hyperfine -L "variation" ,_plain -w5  --export-json $@ -n "cpp-effects" "output/cpp-effects{variation}"

output/bench-cppcoro.json: $(all_cppcoro) | output
	hyperfine -L "variation" _async_generator,_recursive_generator -w5  --export-json $@ -n "cppcoro" "output/cppcoro{variation}"

output/bench-libhandler.json: $(all_libhandler) | output
	hyperfine -L "variation" _tail_noop,_tail,_general -w5  --export-json $@ -n "libhandler" "output/libhandler{variation}"

output/bench-libmpeff.json: $(all_libmpeff) | output
	hyperfine -L "variation" _tail_noop,_tail,_scoped_once,_scoped,_once,_multi -w5  --export-json $@ -n "libmpeff" "output/libmpeff{variation}"

output/bench-libmprompt.json: $(all_libmprompt) | output
	hyperfine -L "variation" "" -w5  --export-json $@ -n "libmprompt" "output/libmprompt{variation}"

output/bench-libco.json: $(all_libco) | output
	hyperfine -L "variation" "" -w5  --export-json $@ -n "libco" "output/libco{variation}"


output/state.png: graph.py output/bench-libseff.json output/bench-native.json output/bench-cpp-effects.json output/bench-libhandler.json output/bench-libmpeff.json output/bench-libmprompt.json output/bench-cppcoro.json output/bench-libco.json | output
	cd ../.. ; python3 -m bench.state_bench.graph

output/state.tex: graph.py output/bench-libseff.json output/bench-native.json output/bench-cpp-effects.json output/bench-libhandler.json output/bench-libmpeff.json output/bench-libmprompt.json output/bench-cppcoro.json output/bench-libco.json | output
	cd ../.. ; python3 -m bench.state_bench.graph

bench: output/bench-libseff.json output/bench-native.json output/bench-cpp-effects.json output/bench-libhandler.json output/bench-libmpeff.json output/bench-libmprompt.json output/bench-cppcoro.json output/bench-libco.json