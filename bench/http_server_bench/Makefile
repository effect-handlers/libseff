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

WRK_TIME = 30

# output/bench-nginx: $(WRK) | output
# 	for th in ${THREADS}; do \
# 		for conn in ${CONNECTIONS}; do \
# 			for rps in ${RPS}; do \
# 				echo Running nginx with $$th threads $$conn connections $$rps rps; \
# 				echo =======; \
# 				sudo nginx -c ~/Repos/libseff/bench/http_server_bench/nginx_$$th.conf; \
# 				running_pid=`cat /run/nginx.pid`; \
# 				sleep 3; \
# 				$(WRK) -t 32 -d${WRK_TIME}s -L -s ${DEPS_DIR}/utils/json.lua -R $$rps -c $$conn http://localhost:8082 > $@-$$conn-$$th-$$rps.txt; \
# 				python3 ./utils/gatherVmPeak.py $${running_pid} >  $@-$$conn-$$th-$$rps.memory; \
# 				sudo nginx -c ~/Repos/libseff/bench/http_server_bench/nginx_$$th.conf -s stop; \
# 				sleep 3; \
# 			done; \
# 		done; \
# 	done;

# sudo nginx -c ~/Repos/libseff/bench/http_server_bench/nginx_$${th}_te.conf & \
# running_pid=$$!;
# sudo kill -s9 $${running_pid}; \

output/bench-nginx_te: $(WRK) | output
	for th in ${THREADS}; do \
		for conn in ${CONNECTIONS}; do \
			for rps in ${RPS}; do \
				echo Running nginx with $$th threads $$conn connections $$rps rps; \
				echo =======; \
				sudo nginx -c ~/Repos/libseff/bench/http_server_bench/nginx_$$th.conf; \
				running_pid=`cat /run/nginx.pid`; \
				sleep 3; \
				$(WRK) -t 32 -d${WRK_TIME}s -L -s ${DEPS_DIR}/utils/json.lua -R $$rps -c $$conn http://localhost:8082 > $@-$$conn-$$th-$$rps.txt; \
				python3 ./utils/gatherVmPeak.py $${running_pid} >  $@-$$conn-$$th-$$rps.memory; \
				sudo nginx -c ~/Repos/libseff/bench/http_server_bench/nginx_$$th.conf -s stop; \
				sleep 3; \
			done; \
		done; \
	done;


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
			$(WRK) -t 32 -d${WRK_TIME}s -L -s ${DEPS_DIR}/utils/json.lua -R $$rps -c $$conn http://localhost:8082 > $@-$$conn-$$th-$$rps.txt; \
			grep VmPeak /proc/$${running_pid}/status >  $@-$$conn-$$th-$$rps.memory; \
			kill $${running_pid}; \
			sleep 1; \
		done; \
	  done; \
	done;

graphs: graph.py | output
	cd ../.. ; python3 -m bench.http_server_bench.graph

bench: output/bench-libscheff output/bench-cohttp_eio output/bench-nethttp_go output/bench-rust_hyper output/bench-nginx_te