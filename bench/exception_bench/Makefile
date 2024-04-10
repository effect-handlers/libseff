include ../common.mk

all_libhandler	:= output/libhandler_noresumex output/libhandler_noresume output/libhandler_tail_noop output/libhandler_tail output/libhandler_scoped  output/libhandler_general
all_libmpeff	:= output/libmpeff_abort output/libmpeff_never output/libmpeff_tail_noop output/libmpeff_tail output/libmpeff_scoped_once output/libmpeff_scoped output/libmpeff_once output/libmpeff_multi

all: output/cpp output/libseff output/libseff_recycle $(all_libhandler) $(all_libmpeff)


output/libseff: libseff.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_recycle: libseff_recycle.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_recycle.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/cpp: cpp.cpp | output
	$(CXX) $(CXXFLAGS) cpp.cpp -o $@ $(LDFLAGS)

# LH_OP_NORESUMEX
output/libhandler_noresumex: libhandler.c $(LIBHANDLER_LIB) | output
	$(CC) $(CFLAGS_LIBHANDLER) -DHANDLER_KIND=LH_OP_NORESUMEX libhandler.c -o $@ $(LDFLAGS_LIBHANDLER)

# LH_OP_NORESUME
output/libhandler_noresume: libhandler.c $(LIBHANDLER_LIB) | output
	$(CC) $(CFLAGS_LIBHANDLER) -DHANDLER_KIND=LH_OP_NORESUME libhandler.c -o $@ $(LDFLAGS_LIBHANDLER)

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

# MPE_OP_ABORT
output/libmpeff_abort: libmpeff.c $(LIBMPEFF_LIB) | output
	$(CC) $(CFLAGS_LIBMPROMPT) -DHANDLER_KIND=MPE_OP_ABORT libmpeff.c -o $@ $(LDFLAGS_LIBMPROMPT)

# MPE_OP_NEVER
output/libmpeff_never: libmpeff.c $(LIBMPEFF_LIB) | output
	$(CC) $(CFLAGS_LIBMPROMPT) -DHANDLER_KIND=MPE_OP_NEVER libmpeff.c -o $@ $(LDFLAGS_LIBMPROMPT)

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

output/bench-libseff.json: output/libseff output/libseff_recycle | output
	hyperfine -L "variation" ,_recycle -w5  --export-json $@ -n "libseff" "output/libseff{variation}"

output/bench-cpp.json: output/cpp | output
	hyperfine -L "variation" "" -w5  --export-json $@ -n "cpp" "output/cpp{variation}"

output/bench-libhandler.json: $(all_libhandler) | output
	hyperfine -L "variation" _noresumex,_noresume,_tail_noop,_tail,_scoped,_general -w5  --export-json $@ -n "libhandler" "output/libhandler{variation}"

output/bench-libmpeff.json: $(all_libmpeff) | output
	hyperfine -L "variation" _abort,_never,_scoped_once,_scoped,_once,_multi -w5  --export-json $@ -n "libmpeff" "output/libmpeff{variation}"
# _tail_noop,_tail have a bug https://github.com/koka-lang/libmprompt/issues/10

bench: output/bench-libseff.json output/bench-cpp.json output/bench-libhandler.json output/bench-libmpeff.json | output

output/exception.png: graph.py bench | output
	cd ../.. ; python3 -m bench.exception_bench.graph
