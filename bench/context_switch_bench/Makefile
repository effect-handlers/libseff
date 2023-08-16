all: output/cppcoro output/libseff output/libco

include ../common.mk

output/libseff: libseff.cpp $(LIBSEFF_LIB) | output
	$(CXX) $(CXXFLAGS_LIBSEFF) libseff.cpp -o $@ $(LDFLAGS_LIBSEFF)

output/libco: libco.cpp $(LIBCO_LIB) | output
	$(CXX) $(CXXFLAGS_LIBCO) libco.cpp -o $@ $(LDFLAGS_LIBCO)

output/cppcoro: cppcoro.cpp $(CPPCORO_LIB) | output
	$(CXX) $(CXXFLAGS_CPPCORO) cppcoro.cpp -o $@ $(LDFLAGS_CPPCORO)
