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
#pragma once

#include <random>

template <typename T>
struct rng {
  std::mt19937_64 generator;
  std::uniform_int_distribution<T> distribution;
  int count;

  rng(unsigned int seed, T from, T to, int count) : distribution(from, to), count(count) {
    generator.seed(seed);
  }

  struct iterator {
    rng& owner;
    int count;

    bool operator != (iterator const& rhs) const {
      return rhs.count != count;
    }

    iterator& operator++() { --count; return *this; }

    T operator* () { return owner.distribution(owner.generator); }

  };

  iterator begin() { return {*this, count}; }
  iterator end() { return {*this, 0}; }
};
