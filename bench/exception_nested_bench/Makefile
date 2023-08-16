all: output/cpp output/libseff output/libseff_norecycle

include ../common.mk

output/libseff: libseff.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff.c -o $@ $(LDFLAGS_LIBSEFF)

output/libseff_norecycle: libseff_norecycle.c $(LIBSEFF_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) libseff_norecycle.c -o $@ $(LDFLAGS_LIBSEFF)

output/cpp: cpp.cpp | output
	$(CXX) $(CXXFLAGS) cpp.cpp -o $@ $(LDFLAGS)
