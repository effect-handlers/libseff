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

BUILD := debug

ROOT_DIR := ../../
DEPS_DIR := ../

CC := clang-10
CXX := clang++-10
PY := python3
LD := $(shell which ld.gold)

FLAGS.debug := -O0
FLAGS.release := -O3 -DNDEBUG -flto=thin

FLAGS := ${FLAGS.${BUILD}} -g -Wall -Wunreachable-code -pedantic -pthread \
	-Wno-gnu-empty-struct \
	-Wno-gnu-zero-variadic-macro-arguments \
	-Wno-gnu-statement-expression \
	-Wno-gnu-empty-initializer \
	-Wno-gnu-folding-constant \
	-Wno-fixed-enum-extension \
	-Wno-return-stack-address \
	-Wno-zero-length-array \
	-Wno-unreachable-code-loop-increment

LIBSEFF_LINK_LIBS := ${ROOT_DIR}/output/lib/libutils.a ${ROOT_DIR}/output/lib/libseff.a

LIBSEFF_INCLUDE_DIRS     := -I${ROOT_DIR}/src -I${ROOT_DIR}/utils -I${ROOT_DIR}/scheduler
LIBMPROMPT_INCLUDE_DIRS  := -I${DEPS_DIR}/libmprompt/include
LIBHANDLER_INCLUDE_DIRS  := -I${DEPS_DIR}/libhandler/inc
CPPCORO_INCLUDE_DIRS     := -I${DEPS_DIR}/cppcoro/include
LIBCO_INCLUDE_DIRS       := -I${DEPS_DIR}/libco
CPP-EFFECTS_INCLUDE_DIRS := -I${DEPS_DIR}/cpp-effects/include
PICOHTTP_INCLUDE_DIRS	 := -I${DEPS_DIR}/picohttpparser

CFLAGS            := $(FLAGS) -std=gnu11
CFLAGS_LIBSEFF    := $(CFLAGS) $(LIBSEFF_INCLUDE_DIRS) -fsplit-stack
CFLAGS_LIBMPROMPT := $(CFLAGS) $(LIBMPROMPT_INCLUDE_DIRS)
CFLAGS_LIBHANDLER := $(CFLAGS) $(LIBHANDLER_INCLUDE_DIRS)

CXXFLAGS             := $(FLAGS) -stdlib=libc++
CXXFLAGS_LIBSEFF     := $(CXXFLAGS) -std=c++20 $(LIBSEFF_INCLUDE_DIRS) -fsplit-stack
CXXFLAGS_CPPCORO     := $(CXXFLAGS) -std=c++20 $(CPPCORO_INCLUDE_DIRS)
# Beware: libco uses the name co_yield, which clashes with C++20's co_yield keyword
CXXFLAGS_LIBCO       := $(CXXFLAGS) -std=c++17 $(LIBCO_INCLUDE_DIRS)
CXXFLAGS_CPP-EFFECTS := $(CXXFLAGS) -std=c++20 $(CPP-EFFECTS_INCLUDE_DIRS)

LDFLAGS             :=  -L${ROOT_DIR}/output -fuse-ld=$(LD) -flto=thin
LDFLAGS_LIBSEFF     := $(LIBSEFF_LINK_LIBS) $(LDFLAGS)
LDFLAGS_LIBCO       := -L${DEPS_DIR}/libco/lib $(LDFLAGS) -ldl -lcolib
LDFLAGS_CPPCORO     := -L${DEPS_DIR}/cppcoro/build/ $(LDFLAGS) -lcppcoro
LDFLAGS_LIBMPROMPT  :=  -L${DEPS_DIR}/libmprompt/out $(LDFLAGS) -lmprompt -lmpeff -lpthread
LDFLAGS_LIBHANDLER   = -L${DEPS_DIR}/libhandler/out/$(shell cat ${LIBHANDLER_CONFIG_NAME})/${BUILD} $(LDFLAGS) -lhandler
LDFLAGS_CPP-EFFECTS := $(LDFLAGS) -lboost_context

.PHONY: all clean libmprompt

output:
	mkdir $@

clean:
	rm -rf output

clean_deps:
	$(MAKE) -C ${DEPS_DIR}/libco clean
	rm -rf ${DEPS_DIR}/cppcoro/build
	$(MAKE) -C ${DEPS_DIR}/libhandler clean
	rm -rf ${DEPS_DIR}/picohttpparser/build
	$(MAKE) -C ${DEPS_DIR}/wrk2 clean

${DEPS_DIR}/cppcoro/tools/cake/src/run.py:
	git submodule update --init --recursive
CPPCORO_LIB := ${DEPS_DIR}/cppcoro/build/libcppcoro.a
$(CPPCORO_LIB): ${DEPS_DIR}/cppcoro/tools/cake/src/run.py
	cd ${DEPS_DIR}/cppcoro; \
	python2.7 tools/cake/src/run.py --debug=run release=optimised --clang-executable=$(CC) lib/build.cake; \
	cp build/*/lib/*.a build/

LIBCO_LIB := ${DEPS_DIR}/libco/lib/libcolib.a
$(LIBCO_LIB):
	$(MAKE) v=release -C ${DEPS_DIR}/libco all

LIBMPEFF_LIB := ${DEPS_DIR}/libmprompt/out/libmpeff.a
LIBMPROMPT_LIB := ${DEPS_DIR}/libmprompt/out/libmprompt.a

$(LIBMPEFF_LIB): libmprompt
$(LIBMPROMPT_LIB): libmprompt

libmprompt:
	cd ${DEPS_DIR}/libmprompt; \
	mkdir -p out; \
	cd out; \
	cmake ../ -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX} -DMP_USE_C=ON; \
	make

# Configure for target: clang, amd64-unknown-linux-gnu
LIBHANDLER_CONFIG_NAME := ${DEPS_DIR}/libhandler/out/config.txt
LIBHANDLER_LIB = ${DEPS_DIR}/libhandler/out/$(shell cat ${LIBHANDLER_CONFIG_NAME})/${BUILD}/libhandler.a

$(LIBHANDLER_LIB):
	cd ${DEPS_DIR}/libhandler; \
	./configure --cc=${CC} --cxx=${CXX} | awk '/Configure for target:/ { printf "%s-%s\n", substr($$4, 1, length($$4)-1), $$5 }' > ${DEPS_DIR}/libhandler/tmp_config.txt
	$(MAKE) VARIANT=${BUILD} -C ${DEPS_DIR}/libhandler depend
	$(MAKE) VARIANT=${BUILD} -C ${DEPS_DIR}/libhandler
	mv ${DEPS_DIR}/libhandler/tmp_config.txt ${DEPS_DIR}/libhandler/out/config.txt

PICOHTTP_LIB := ${DEPS_DIR}/picohttpparser/build/picohttpparser.a

${DEPS_DIR}/picohttpparser/build:
	mkdir -p $@

${DEPS_DIR}/picohttpparser/build/picohttpparser.o: ${DEPS_DIR}/picohttpparser/picohttpparser.c ${DEPS_DIR}/picohttpparser/picohttpparser.h | ${DEPS_DIR}/picohttpparser/build
	$(CC) $(CFLAGS) -o $@ -c $<

$(PICOHTTP_LIB): ${DEPS_DIR}/picohttpparser/build/picohttpparser.o | ${DEPS_DIR}/picohttpparser/build
	llvm-ar-12 -rcs $@ $<

WRK := ${DEPS_DIR}/wrk2/wrk

$(WRK): ${DEPS_DIR}/wrk2
	$(MAKE) -C $<
