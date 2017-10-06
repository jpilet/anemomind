/*
 * transducerDemo.cpp
 *
 *  Created on: 28 Sep 2017
 *      Author: jonas
 */

#include <iostream>
#include <vector>
#include <server/common/Transducer.h>

// Small test program to test the transducer implementation
//
// clang++ -S -std=c++11 -I../../ -O3 -DUSE_TRANSDUCER transducerDemo.cpp
//
// Omit the -S to get an executable.

/*
 * Without transducers:
 *
 *     0.912626 seconds
 *
 * With first implementation:
 *
 *     4.18948 seconds
 *
 * With revised implementation (const step):
 *
 *     0.977381 seconds
 *
 * With non-const steps:
 *
 *    0.916209 seconds
 *
 */

using namespace sail;

#ifdef USE_TRANSDUCER
const bool useTransducer = true;
#else
const bool useTransducer = false;
#endif

template <bool ut> struct SumOddDiv3;

template <>
struct SumOddDiv3<true> {
  static int apply(const std::vector<int>& data) {

    auto T =
        sail::trFilter([](int i) {return i % 2 == 1;})
        | sail::trMap([](int i) {return i/3;});

    return reduce(
        T.apply(AddStep<int>()),
        0, data);
  }
};

template <>
struct SumOddDiv3<false> {
  static int apply(const std::vector<int>& data) {
    int sum = 0;
    for (auto x: data) {
      if (x % 2 == 1) {
        sum += x/3;
      }
    }
    return sum;
  }
};


int sumOddDiv3() {
  const int m = 30000;
  const int n = 30000;
  std::vector<int> data(n);
  for (int i = 0; i < n; i++) {
    data[i] = i;
  }
  auto start = std::chrono::system_clock::now();
  int total = 0;
  for (int i = 0; i < m; i++) {
    total += SumOddDiv3<useTransducer>::apply(data) % 3;
  }
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsedSeconds = end - start;
  std::cout << "Elapsed seconds: " << elapsedSeconds.count() << std::endl;
  return total;
}

int main() {
  std::cout << (useTransducer?
      "USING TRANSDUCER " : "USING FOR LOOP") << std::endl;
  std::cout << "Sum is " << sumOddDiv3() << std::endl;
  return 0;
}


