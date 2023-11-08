all: output/cpp output/libseff output/libseff_recycle

include ../common.mk

output/libseff: libseff.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libseff_recycle: libseff_recycle.c $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) libseff_recycle.c -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/cpp: cpp.cpp | output
	$(CXX) $(CXXFLAGS) cpp.cpp -o $@ $(LDFLAGS)
