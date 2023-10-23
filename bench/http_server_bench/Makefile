all: output/libscheff

include ../common.mk

output/%: %.c $(LIBSEFF_LIB) $(PICOHTTP_LIB) | output
	$(CC) $(CFLAGS_LIBSEFF) $(PICOHTTP_INCLUDE_DIRS) $< $(PICOHTTP_LIB) -o $@ $(LDFLAGS_LIBSEFF)

output/cohttp_eio: external/cohttp_eio | output
	cp $< $@

output/nethttp_go: external/nethttp_go | output
	cp $< $@

output/rust_hyper: external/rust_hyper | output
	cp $< $@

.PHONY: bench

THREADS	= 8
CONNECTIONS = 100 1000 5000 10000 50000
RPS =  800000

# THREADS	= 1 8 16 32
# CONNECTIONS = 1000
# RPS = 2000000

# THREADS	= 1 8 16 32
# CONNECTIONS = 100 1000 5000 10000 50000
# RPS = 35000 50000 100000 200000 400000 800000 1500000 2000000

FORCE:
output/bench-%: output/% FORCE $(WRK) | output
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
			kill $${running_pid}; \
			sleep 1; \
		done; \
	  done; \
	done;
	# hyperfine -L "stack size (bytes)" 0,1000,2000,3000,4000,5000 -w5  --export-json $@ -n "$*" "$< 10000000 0 {stack size (bytes)}"

bench: output/bench-libscheff output/bench-cohttp_eio output/bench-nethttp_go output/bench-rust_hyper