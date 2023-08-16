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
#include "runner.hpp"

template <typename Iterator>
bool naive_binary_search(Iterator first, Iterator last, int val) {
  auto len = last - first;
  while (len > 0) {
    const auto half = len / 2;
    const auto middle = first + half;
    const auto middle_key = *middle;
    if (middle_key < val) {
      first = middle + 1;
      len = len - half - 1;
    } else {
      len = half;
    }
    if (middle_key == val)
      return true;
  }
  return false;
}

static long testNaive(State &s) {
  size_t found = 0;
  auto beg = s.v.begin();
  auto end = s.v.end();
  for (int key : s.lookups)
    if (naive_binary_search(beg, end, key))
      ++found;
  return found;
}

int main(int argc, const char** argv) {
    int streams = 1;

    if (argc == 2){
      streams = atoi(argv[1]);
    }
  return runner(testNaive, Case::big, streams, "naive");
}
