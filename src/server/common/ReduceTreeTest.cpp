/*
 * ReduceTreeTest.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/ReduceTree.h>

using namespace sail;

TEST(ReduceTreeTest, SumTree) {
  Array<int> numbers{1, 2, 3, 4, 5};

  ReduceTree<int> tree(
      [](int a, int b) {return a + b;}, numbers);

  EXPECT_EQ(tree.top(), 1 + 2 + 3 + 4 + 5);

  for (auto x: tree.allData()) {
    std::cout << " x = " << x << std::endl;
  }
}

