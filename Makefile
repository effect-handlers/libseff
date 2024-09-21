#
# Copyright (c) 2023 Huawei Technologies Co., Ltd.
#
# libseff is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
# 	    http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
#

BUILD        := debug
STACK_POLICY := segmented
ARCH         := x86-64

CC  := clang-10
CXX := clang++-10
PY  := python3
LD  := $(shell which ld.gold)

FLAGS.debug   := -O0
FLAGS.release := -O3 -DNDEBUG
FLAGS         := ${FLAGS.${BUILD}} -g -Wall -Wunreachable-code

CFLAGSCOMMON := $(FLAGS) -pedantic -pthread \
	-Wno-gnu-empty-struct \
	-Wno-gnu-zero-variadic-macro-arguments \
	-Wno-gnu-statement-expression \
	-Wno-gnu-empty-initializer \
	-Wno-gnu-folding-constant \
	-Wno-fixed-enum-extension

CFLAGS.segmented := -fsplit-stack
CFLAGS.vmmem     :=
CFLAGS.fixed     :=
CFLAGS           := $(CFLAGSCOMMON) -std=gnu11 ${CFLAGS.${STACK_POLICY}}

CXXFLAGS.segmented := -fsplit-stack
CXXFLAGS.vmmem     :=
CXXFLAGS.fixed     :=
CXXFLAGS           := $(FLAGS) -std=c++17 ${CXXFLAGS.${STACK_POLICY}}

LDFLAGS.segmented :=
LDFLAGS.vmmem     :=
LDFLAGS.fixed     :=
LDFLAGS           := -fuse-ld=$(LD) -L./output ${LDFLAGS.${STACK_POLICY}}

.PHONY: all test bench clean

all: output/lib/libseff.a test bench

GENERATE_PY := $(PY) decls/seff_types.py \
	--arch ${ARCH} \
	--stack ${STACK_POLICY}

src/seff_types.h: decls/seff_types.py decls/generate.py
	$(GENERATE_PY)

asm/seff_types.S: decls/seff_types.py decls/generate.py
	$(GENERATE_PY)

output/seff.o: src/seff.c src/seff.h src/seff_types.h | output
	$(CC) $(CFLAGS) -I./src -o output/seff.o -c src/seff.c

output/seff_asm.o: asm/seff-${ARCH}.S asm/seff_types.S | output
	$(CC) $(CFLAGS) -I./asm -o output/seff_asm.o -c asm/seff-${ARCH}.S

output/seff_mem.o: src/mem/seff_mem_${STACK_POLICY}.c src/mem/seff_mem.h src/seff_types.h | output
	$(CC) $(CFLAGS) -I./src -o output/seff_mem.o -c src/mem/seff_mem_${STACK_POLICY}.c

output/seff_mem_asm.o: src/mem/seff_mem_${STACK_POLICY}.S | output
	$(CC) $(CFLAGS) -I./src -o output/seff_mem_asm.o -c src/mem/seff_mem_${STACK_POLICY}.S


output/actors.o: src/seff.h src/seff_types.h scheduler/scheff.h utils/actors.h utils/actors.c | output/lib
	$(CC) $(CFLAGS) -I./src -I./utils -I./scheduler -o output/actors.o -c utils/actors.c

output/cl_queue.o: utils/cl_queue.h utils/cl_queue.c | output/lib
	$(CC) $(CFLAGS) -I./utils -o output/cl_queue.o -c utils/cl_queue.c

output/net.o: utils/net.c utils/net.h | output/lib
	$(CC) $(CFLAGS) -I./src -I./utils -I./scheduler -o $@ -c $<

output/http_response.o: utils/http_response.c utils/http_response.h | output/lib
	$(CC) $(CFLAGS) -o $@ -c $<

output/tk_queue.o: utils/tk_queue.h utils/tk_queue.c | output/lib
	$(CC) $(CFLAGS) -I./utils -o output/tk_queue.o -c utils/tk_queue.c

output/tl_queue.o: utils/tl_queue.h utils/tl_queue.c | output/lib
	$(CC) $(CFLAGS) -I./utils -o output/tl_queue.o -c utils/tl_queue.c

output/scheff.o: scheduler/scheff.h scheduler/scheff.c | output/lib
	$(CC) $(CFLAGS) -I./src -I./utils -o output/scheff.o -c scheduler/scheff.c

output/tests/%: tests/%.c output/lib/libutils.a output/lib/libseff.a | output/tests
	$(CC) $(CFLAGS) -I./src -I./utils -I./scheduler -o $@.o -c $<
	$(CC) $(CFLAGS) -I./src -I./utils -I./scheduler -o $@ $@.o output/lib/libutils.a output/lib/libseff.a $(LDFLAGS)

output/tests/%: tests/%.cpp output/lib/libutils.a output/lib/libseff.a | output/tests
	$(CXX) $(CXXFLAGS) -I./src -o $@.o -c $<
	$(CXX) $(CXXFLAGS) -I./src -o $@ $@.o output/lib/libseff.a output/lib/libutils.a -pthread $(LDFLAGS)

test: output/lib/libseff.a output/lib/libutils.a ./tests/*.c
	for test in tests/*.c ; do \
		$(MAKE) output/$${test%%.*} ; \
	done

bench: output/lib/libseff.a output/lib/libutils.a
	for bench_dir in bench/*_bench/ ; do \
		$(MAKE) BUILD=${BUILD} -C $${bench_dir} all ; \
	done

bench/%: output/lib/libseff.a output/lib/libutils.a
	$(MAKE) BUILD=${BUILD} -C $@ all

compile_commands.json:
	$(MAKE) --always-make --dry-run \
	 | grep -wE 'gcc|g\+\+|clang' \
	 | grep -w '\-c' \
	 | jq -nR '[inputs|{directory:"$(shell pwd)", command:., file: match(" [^ ]+$$").string[1:]}]' \
	 > compile_commands.json

output:
	mkdir $@

output/lib: | output
	mkdir $@

output/tests: | output
	mkdir $@

output/bench: | output
	mkdir $@

clean:
	rm -rf output
	rm -f asm/seff_types.S
	rm -f src/seff_types.h
	for bench_dir in bench/*_bench/ ; do \
		$(MAKE) -C $${bench_dir} clean ; \
	done

output/lib/libseff.a: output/seff_mem.o output/seff_mem_asm.o output/seff.o output/seff_asm.o | output/lib
	ar -rcs output/lib/libseff.a output/seff_mem.o output/seff_mem_asm.o output/seff.o output/seff_asm.o

output/lib/libutils.a: output/actors.o output/cl_queue.o output/tk_queue.o output/tl_queue.o output/scheff.o output/net.o output/http_response.o  | output/lib
	ar -rcs $@ $^
