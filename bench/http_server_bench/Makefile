all: output/libscheff output/cohttp_eio output/nethttp_go output/rust_hyper

include ../common.mk

output/%: %.c $(LIBSEFF_SEGMENTED_LINK_LIBS) $(PICOHTTP_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF_SEGMENTED) $(PICOHTTP_INCLUDE_DIRS) $< $(PICOHTTP_LIB) -o $@ $(LDFLAGS_LIBSEFF_SEGMENTED)

output/cohttp_eio: external/retro-httpaf-bench/cohttp-eio | output
	cd external/retro-httpaf-bench/cohttp-eio 	; \
	./build.sh 									; \
	ls 											; \
	cd ../../.. 								; \
	cp external/retro-httpaf-bench/cohttp-eio/cohttp_eio.exe $@

output/nethttp_go: external/retro-httpaf-bench/nethttp-go | output
	cd external/retro-httpaf-bench/nethttp-go 	; \
	./build.sh 									; \
	ls 											; \
	cd ../../.. 								; \
	cp external/retro-httpaf-bench/nethttp-go/nethttp_go.exe $@

output/rust_hyper: external/retro-httpaf-bench/rust-hyper | output
	cd external/retro-httpaf-bench/rust-hyper 	; \
	./build.sh 									; \
	ls 											; \
	cd ../../.. 								; \
	cp external/retro-httpaf-bench/rust-hyper/rust_hyper.exe $@

.PHONY: bench

# THREADS	= 8
# CONNECTIONS = 100 1000 5000 10000 50000
# RPS =  800000

# THREADS	= 1 8 16 32
# CONNECTIONS = 1000
# RPS = 2000000

# THREADS	= 1 8 16 32
# CONNECTIONS = 100 1000 5000 10000 50000
# RPS = 35000 50000 100000 200000 400000 800000 1500000 2000000

THREADS	= 1 8 16
CONNECTIONS = 1000 5000 10000
RPS = 50000 100000 500000 1000000 1500000 2000000


FORCE:
output/bench-%: output/% $(WRK) | output
	for th in ${THREADS}; do \
      export GOMAXPROCS=$$th; \
      export COHTTP_DOMAINS=$$th; \
      export HTTPAF_EIO_DOMAINS=$$th; \
      export RUST_CORES=$$th; \
      export LIBSEFF_THREADS=$$th; \
	  for conn in ${CONNECTIONS}; do \
	    for rps in ${RPS}; do \
			echo Running $< with $$th threads $$conn connections $$rps rps; \
			echo =======; \
			./$< &  \
			running_pid=$$!; \
			sleep 2; \
			$(WRK) -t 32 -d30s -L -s ${DEPS_DIR}/utils/json.lua -R $$rps -c $$conn http://localhost:8082 > $@-$$conn-$$th-$$rps.txt; \
			grep VmPeak /proc/$${running_pid}/status >  $@-$$conn-$$th-$$rps.memory; \
			kill $${running_pid}; \
			sleep 1; \
		done; \
	  done; \
	done;

output/http_%.png: graph.py | output
	cd ../.. ; python3 -m bench.http_server_bench.graph

graphs: output/http_threads.png output/http_conn.png output/http_rps_1.png output/http_rps_8.png output/http_rps_16.png output/http_rps_32.png output/http_memory.png

bench: output/bench-libscheff output/bench-cohttp_eio output/bench-nethttp_go output/bench-rust_hyper