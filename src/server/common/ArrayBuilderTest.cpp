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

TEST(ArrayBuilderTest, CommonTest) {
  ArrayBuilder<int> builder;
  EXPECT_TRUE(builder.get().empty());
  builder.add(119);
  builder.add(60);
  builder.add(-3);
  Arrayi data = builder.get();
  EXPECT_EQ(data.size(), 3);
  EXPECT_EQ(data[0], 119);
  EXPECT_EQ(data[1], 60);
  EXPECT_EQ(data[2], -3);
}


