all: output/libseff output/libseff_direct output/c output/cpp-effects output/libhandler

include ../common.mk

output/libseff: libseff.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff.c -o $@ $(LDFLAGS_LIBSEFF)

output/libseff_direct: libseff_direct.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff_direct.c -o $@ $(LDFLAGS_LIBSEFF)

output/c: c.c | output
	$(CC) $(CFLAGS) c.c -o $@ $(LDFLAGS)

output/cpp-effects: cpp-effects.cpp | output
	$(CXX) $(CXXFLAGS_CPP-EFFECTS) cpp-effects.cpp -o $@ $(LDFLAGS_CPP-EFFECTS)

output/libhandler: libhandler.c $(LIBHANDLER_LIB) | output
	$(CC) $(CFLAGS_LIBHANDLER) libhandler.c -o $@ $(LDFLAGS_LIBHANDLER)
