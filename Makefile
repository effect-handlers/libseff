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

OUTPUT_DIR    := output-${STACK_POLICY}
INCLUDE_DIRS  := -I./src -I./${OUTPUT_DIR}/include

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
LDFLAGS           := -fuse-ld=$(LD) -L./$(OUTPUT_DIR) ${LDFLAGS.${STACK_POLICY}}

.PHONY: all test bench clean

all: $(OUTPUT_DIR)/lib/libseff.a test

GENERATE_PY := $(PY) decls/seff_types.py \
	--arch ${ARCH}                       \
	--stack ${STACK_POLICY}              \
	--target-dir $(OUTPUT_DIR)/include

$(OUTPUT_DIR)/include/seff_types.h: decls/seff_types.py decls/generate.py | $(OUTPUT_DIR)/include
	$(GENERATE_PY)

$(OUTPUT_DIR)/include/seff_types.S: decls/seff_types.py decls/generate.py | $(OUTPUT_DIR)/include
	$(GENERATE_PY)

$(OUTPUT_DIR)/seff.o: src/seff.c src/seff.h $(OUTPUT_DIR)/include/seff_types.h | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) ${INCLUDE_DIRS} -o $(OUTPUT_DIR)/seff.o -c src/seff.c

$(OUTPUT_DIR)/seff_asm.o: asm/seff.S $(OUTPUT_DIR)/include/seff_types.S | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) ${INCLUDE_DIRS} -o $(OUTPUT_DIR)/seff_asm.o -c asm/seff.S

$(OUTPUT_DIR)/seff_mem.o: src/mem/seff_mem_${STACK_POLICY}.c src/mem/seff_mem.h $(OUTPUT_DIR)/include/seff_types.h | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) ${INCLUDE_DIRS} -o $(OUTPUT_DIR)/seff_mem.o -c src/mem/seff_mem_${STACK_POLICY}.c

$(OUTPUT_DIR)/seff_mem_asm.o: src/mem/seff_mem_${STACK_POLICY}.S | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) ${INCLUDE_DIRS} -o $(OUTPUT_DIR)/seff_mem_asm.o -c src/mem/seff_mem_${STACK_POLICY}.S


$(OUTPUT_DIR)/actors.o: src/seff.h $(OUTPUT_DIR)/include/seff_types.h scheduler/scheff.h utils/actors.h utils/actors.c | $(OUTPUT_DIR)/lib
	$(CC) $(CFLAGS) ${INCLUDE_DIRS} -I./utils -I./scheduler -o $(OUTPUT_DIR)/actors.o -c utils/actors.c

$(OUTPUT_DIR)/cl_queue.o: utils/cl_queue.h utils/cl_queue.c | $(OUTPUT_DIR)/lib
	$(CC) $(CFLAGS) -I./utils -o $(OUTPUT_DIR)/cl_queue.o -c utils/cl_queue.c

$(OUTPUT_DIR)/net.o: utils/net.c utils/net.h | $(OUTPUT_DIR)/lib
	$(CC) $(CFLAGS) ${INCLUDE_DIRS} -I./utils -I./scheduler -o $@ -c $<

$(OUTPUT_DIR)/http_response.o: utils/http_response.c utils/http_response.h | $(OUTPUT_DIR)/lib
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTPUT_DIR)/tk_queue.o: utils/tk_queue.h utils/tk_queue.c | $(OUTPUT_DIR)/lib
	$(CC) $(CFLAGS) -I./utils -o $(OUTPUT_DIR)/tk_queue.o -c utils/tk_queue.c

$(OUTPUT_DIR)/tl_queue.o: utils/tl_queue.h utils/tl_queue.c | $(OUTPUT_DIR)/lib
	$(CC) $(CFLAGS) -I./utils -o $(OUTPUT_DIR)/tl_queue.o -c utils/tl_queue.c

$(OUTPUT_DIR)/scheff.o: scheduler/scheff.h scheduler/scheff.c | $(OUTPUT_DIR)/lib
	$(CC) $(CFLAGS) ${INCLUDE_DIRS} -I./utils -o $(OUTPUT_DIR)/scheff.o -c scheduler/scheff.c

$(OUTPUT_DIR)/tests/%: tests/%.c $(OUTPUT_DIR)/lib/libutils.a $(OUTPUT_DIR)/lib/libseff.a | $(OUTPUT_DIR)/tests
	$(CC) $(CFLAGS) ${INCLUDE_DIRS} -I./utils -I./scheduler -o $@.o -c $<
	$(CC) $(CFLAGS) ${INCLUDE_DIRS} -I./utils -I./scheduler -o $@ $@.o $(OUTPUT_DIR)/lib/libutils.a $(OUTPUT_DIR)/lib/libseff.a $(LDFLAGS)

$(OUTPUT_DIR)/tests/%: tests/%.cpp $(OUTPUT_DIR)/lib/libutils.a $(OUTPUT_DIR)/lib/libseff.a | $(OUTPUT_DIR)/tests
	$(CXX) $(CXXFLAGS) ${INCLUDE_DIRS} -o $@.o -c $<
	$(CXX) $(CXXFLAGS) ${INCLUDE_DIRS} -o $@ $@.o $(OUTPUT_DIR)/lib/libseff.a $(OUTPUT_DIR)/lib/libutils.a -pthread $(LDFLAGS)

test: $(OUTPUT_DIR)/lib/libseff.a $(OUTPUT_DIR)/lib/libutils.a ./tests/*.c
	for test in tests/*.c ; do \
		$(MAKE) $(OUTPUT_DIR)/$${test%%.*} ; \
	done

bench: $(OUTPUT_DIR)/lib/libseff.a $(OUTPUT_DIR)/lib/libutils.a
	for bench_dir in bench/*_bench/ ; do \
		$(MAKE) BUILD=${BUILD} -C $${bench_dir} all ; \
	done

bench/%: $(OUTPUT_DIR)/lib/libseff.a $(OUTPUT_DIR)/lib/libutils.a
	$(MAKE) BUILD=${BUILD} -C $@ all

compile_commands.json:
	$(MAKE) --always-make --dry-run \
	 | grep -wE 'gcc|g\+\+|clang' \
	 | grep -w '\-c' \
	 | jq -nR '[inputs|{directory:"$(shell pwd)", command:., file: match(" [^ ]+$$").string[1:]}]' \
	 > compile_commands.json

$(OUTPUT_DIR):
	mkdir $@

$(OUTPUT_DIR)/lib: | $(OUTPUT_DIR)
	mkdir $@

$(OUTPUT_DIR)/tests: | $(OUTPUT_DIR)
	mkdir $@

$(OUTPUT_DIR)/bench: | $(OUTPUT_DIR)
	mkdir $@

$(OUTPUT_DIR)/include: | $(OUTPUT_DIR)
	mkdir $@

clean:
	rm -rf $(OUTPUT_DIR)
	for bench_dir in bench/*_bench/ ; do \
		$(MAKE) -C $${bench_dir} clean ; \
	done

$(OUTPUT_DIR)/lib/libseff.a: $(OUTPUT_DIR)/seff_mem.o $(OUTPUT_DIR)/seff_mem_asm.o $(OUTPUT_DIR)/seff.o $(OUTPUT_DIR)/seff_asm.o | $(OUTPUT_DIR)/lib
	ar -rcs $(OUTPUT_DIR)/lib/libseff.a $(OUTPUT_DIR)/seff_mem.o $(OUTPUT_DIR)/seff_mem_asm.o $(OUTPUT_DIR)/seff.o $(OUTPUT_DIR)/seff_asm.o

$(OUTPUT_DIR)/lib/libutils.a: $(OUTPUT_DIR)/actors.o $(OUTPUT_DIR)/cl_queue.o $(OUTPUT_DIR)/tk_queue.o $(OUTPUT_DIR)/tl_queue.o $(OUTPUT_DIR)/scheff.o $(OUTPUT_DIR)/net.o $(OUTPUT_DIR)/http_response.o  | $(OUTPUT_DIR)/lib
	ar -rcs $@ $^
