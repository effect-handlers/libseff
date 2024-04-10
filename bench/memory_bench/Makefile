all: output/cppcoro output/libseff output/libco

include ../common.mk

output/libseff: libseff.cpp $(LIBSEFF_SEGMENTED_LINK_LIBS) | output
	$(CXX) $(CXXFLAGS_LIBSEFF_SEGMENTED) libseff.cpp -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/libco: libco.cpp $(LIBCO_LIB) | output
	$(CXX) $(CXXFLAGS_LIBCO) libco.cpp -o $@ $(LDFLAGS_LIBCO)

output/cppcoro: cppcoro.cpp $(CPPCORO_LIB) | output
	$(CXX) $(CXXFLAGS_CPPCORO) cppcoro.cpp -o $@ $(LDFLAGS_CPPCORO)


.PHONY: bench

output/bench-%.yaml: output/% | output
	rm $@; \
	touch $@; \
	# echo "{" >> $@; \
	for i in ${PADDINGS}; 				\
		do \
		echo -n "$$i: " >> $@; \
		valgrind $< 10000 0 $$i 2>&1 | awk '/total heap usage/ { print $$9}' | sed s/,//g >> $@; \
		# echo , >> $@ ; \
	done; \
	# echo "}" >> $@;


bench: output/bench-libseff.yaml output/bench-libco.yaml output/bench-cppcoro.yaml

output/memory.png: output/bench-libseff.yaml output/bench-libco.yaml output/bench-cppcoro.yaml | output
	cd ../.. ; python3 -m bench.memory_bench.graph

PADDINGS = 0 100 500 1000 3000 5000 10000 20000 30000 40000 50000 60000 70000 80000 90000 100000
# PADDINGS = 0 100 200 300 400 500 600 700 800 900 1000 2000 3000 4000 5000 6000 7000 8000 9000 10000 11000 12000 13000 14000 15000 16000 17000 18000 19000 20000 21000 22000 23000 24000 25000 26000 27000 28000 29000 30000 31000 32000 33000 34000 35000 36000 37000 38000 39000 40000 41000 42000 43000 44000 45000 46000 47000 48000 49000 50000 60000 70000 80000 90000 100000