all: all_libseff all_native all_cpp-effects all_libhandler all_libmpeff all_libmprompt
all_libseff: output/libseff output/libseff_direct
all_native: output/c
all_cpp-effects: output/cpp-effects
all_libhandler: output/libhandler_tail_noop output/libhandler_tail  output/libhandler_general
# output/libhandler_scoped Removed since there appears to be a bug
all_libmpeff: output/libmpeff_tail_noop output/libmpeff_tail output/libmpeff_scoped_once output/libmpeff_scoped output/libmpeff_once output/libmpeff_multi
all_libmprompt: output/libmprompt

include ../common.mk

output/libseff: libseff.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff.c -o $@ $(LDFLAGS_LIBSEFF)

output/libseff_direct: libseff_direct.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff_direct.c -o $@ $(LDFLAGS_LIBSEFF)

output/c: c.c | output
	$(CC) $(CFLAGS) c.c -o $@ $(LDFLAGS)

output/cpp-effects: cpp-effects.cpp | output
	$(CXX) $(CXXFLAGS_CPP-EFFECTS) cpp-effects.cpp -o $@ $(LDFLAGS_CPP-EFFECTS)

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