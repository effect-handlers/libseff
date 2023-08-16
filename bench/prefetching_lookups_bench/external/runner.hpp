/*Copyright (c) 2018 Gor Nishanov All rights reserved.

This code is licensed under the MIT License (MIT).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

From https://github.com/GorNishanov/await
*/
#include "rng.h"
#include <chrono>
#include <ratio>
#include <stdio.h>
#include <vector>
#include <string_view>
#include <functional>

// Test state.
struct State {
  std::vector<int> v;
  std::vector<int> lookups;
  int repeat;

  int streams;
  char const *algo_name;

  State(int ByteCount, int LookupCount, int Repeat) : repeat(Repeat) {
    auto seed1 = 0;

    int count = (ByteCount / sizeof(int));
    v.reserve(count);
    for (int i = 0; i < count; ++i)
      v.push_back(i + i);

    lookups.reserve(LookupCount);
    for (auto i : rng<int>(seed1, 0, count + count, LookupCount))
      lookups.push_back(i);

    printf("count: %d lookups: %d repeat %d\n", count, LookupCount, Repeat);
  }

  using hrc_clock = std::chrono::high_resolution_clock;
  hrc_clock::time_point start_time;

  void start(int streams, const char *algo_name) {
    this->streams = streams;
    this->algo_name = algo_name;
    start_time = hrc_clock::now();
  }

  void stop() {
    auto stop_time = hrc_clock::now();
    std::chrono::duration<double, std::nano> elapsed = stop_time - start_time;

    auto divby = log2((double)v.size());
    auto perop = elapsed.count() / divby / lookups.size() / repeat;

    printf("%g ns per lookup/log2(size)\n", perop);
  }
};

struct TestParam {
  int SizeInBytes;
  int LookupSize;
  int Repeat;

  long ExpectedResult; // sanity check for bugs
};

enum Case { quick, l1, l2, l3, big };

int runner(std::function<long(State&)> testFn, Case testCase, int streams, const char * algo_name) {
  TestParam param;
  switch (testCase) {
  case Case::quick:
    param = TestParam{ 16*1024, 1024, 1,            505};
    break;
  case Case::l1:
    param = TestParam{ 16*1024, 1024, 10000,        5050000};
    break;
  case Case::l2:
    param = TestParam{200*1024, 1024*1024, 50,      26225050};
    break;
  case Case::l3:
    param = TestParam{6*1024*1024, 1024*1024, 50,   26215900};
    break;
  case Case::big:
    param = TestParam{256*1024*1024, 1024*1024, 5,  2624940};
    break;
  }

  State s(param.SizeInBytes, param.LookupSize, param.Repeat);

  s.start(streams, algo_name);

  long sum = 0;
  int repeat = s.repeat;
  while (repeat-- > 0) {
    auto result = testFn(s);
    sum += result;
  }
  s.stop();
  printf("sum %ld\n", sum);
  if (sum != param.ExpectedResult) {
    printf("!!!! BUG, expected %ld\n", param.ExpectedResult);
    return 1;
  }
  return 0;
}
