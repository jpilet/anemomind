/*
 *  Created on: 2014-04-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArrayBuilder.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(ArrayBuilderTest, StdVectorImplementedAsWeAssume) {
  std::vector<int> vec;
  vec.push_back(119);
  vec.push_back(60);
  vec.push_back(-3);
  Array<int> data = Array<int>::referToVector(vec);
  EXPECT_EQ(data[0], vec[0]);
  EXPECT_EQ(data[1], vec[1]);
  EXPECT_EQ(data[2], vec[2]);
}


