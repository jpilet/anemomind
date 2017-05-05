/*
 * ArrayCopyTest.cpp
 *
 *  Created on: 5 May 2017
 *      Author: jonas
 */
#include <gtest/gtest.h>
#include <device/anemobox/LocalArrayCopy.h>
#include <vector>

template <int N>
void checkCopy(const std::vector<int>& src,
    const sail::LocalArrayCopy<int, N>& copy) {
  int counter = 0;
  for (auto x: copy) {
    EXPECT_EQ(x, src[counter]);
    counter++;
  }
  EXPECT_EQ(src.size(), counter);
}

TEST(LocalArrayCopy, TestIt) {
  std::vector<int> myInts{9, 7, 8, 3};
  checkCopy(myInts, sail::LocalArrayCopy<int, 2>(myInts.begin(), myInts.end()));
  checkCopy(myInts, sail::LocalArrayCopy<int, 8>(myInts.begin(), myInts.end()));
}



