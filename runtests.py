#!/usr/bin/python3

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
# Concurrent test runner for libseff
#

import subprocess
from glob import glob
from typing import Set,Tuple

excluded_tests = {
    './output/tests/echo_server',
    './output/tests/many_threads'
}

subprocess.run(["make", "test"])

testcases = [
    test for test in glob("./output/tests/*")
    if not test in excluded_tests and test[-2:] != '.o'
]

longest_testname_length = 0
for test in testcases:
    longest_testname_length = max(longest_testname_length, len(test))

testcase_processes: Set[Tuple[str, subprocess.Popen]]  = set()

for test in testcases:
    testcase_processes.add((
        test, subprocess.Popen(test, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    ))

while len(testcase_processes) > 0:
    remaining = set()
    print(f'   Remaining: {len(testcase_processes)}', end='\r')
    for [test, p] in testcase_processes:
        status = p.poll()
        if status == None:
            remaining.add((test, p))
        else:
            padding = (longest_testname_length + 1 - len(test)) * ' '
            print(f'{test}{padding}{status}')
    testcase_processes = remaining



