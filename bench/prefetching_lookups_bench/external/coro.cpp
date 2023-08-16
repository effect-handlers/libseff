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
#include <vector>
#include <stdio.h>

#include "coro_infra.h"
#include "runner.hpp"

template <typename Iterator, typename Found, typename NotFound>
root_task CoroBinarySearch(Iterator first, Iterator last, int val,
                          Found on_found, NotFound on_not_found) {
  auto len = last - first;
  while (len > 0) {
    auto half = len / 2;
    auto middle = first + half;
    auto x = co_await prefetch(*middle);
    if (x < val) {
      first = middle;
      ++first;
      len = len - half - 1;
    } else
      len = half;
    if (x == val)
      co_return on_found(middle);
  }
  on_not_found();
}

long CoroMultiLookup(
  std::vector<int> const& v, std::vector<int> const& lookups, int streams) {

  size_t found_count = 0;
  size_t not_found_count = 0;

  throttler t(streams);

  for (auto key: lookups)
    t.spawn(CoroBinarySearch(v.begin(), v.end(), key,
      [&](auto) { ++found_count; }, [&] { ++not_found_count; }));

  t.run();

  if (found_count + not_found_count != lookups.size())
    printf("BUG: found %zu, not-found: %zu total %zu\n", found_count,
           not_found_count, found_count + not_found_count);

  return found_count;
}

long testCoro(State& s){
  return CoroMultiLookup(s.v, s.lookups, s.streams);
}

int main(int argc, const char** argv) {
    int streams = 15;

    if (argc == 2){
      streams = atoi(argv[1]);
    }

  return runner(testCoro, Case::big, streams, "coro");
}
