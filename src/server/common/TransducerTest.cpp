/*
 * TransducerTest.cpp
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/Transducer.h>

using namespace sail;

TEST(TransducerTest, MapTest) {
  std::vector<int> input{1, 2, 3, 4, 5, 6};
  std::vector<int> dst;
  auto i = std::inserter(dst, dst.end());
  auto step = iteratorStep(i);
}
