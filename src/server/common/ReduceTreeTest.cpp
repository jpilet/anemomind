/*
 * ReduceTreeTest.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/ReduceTree.h>
#include <server/common/ArrayIO.h>

using namespace sail;

TEST(ReduceTreeTest, SumTree) {
  Array<int> numbers{1, 2, 3, 4, 5};

  ReduceTree<int> tree(
      [](int a, int b) {return a + b;}, numbers);

  EXPECT_EQ(tree.top(), 1 + 2 + 3 + 4 + 5);
  EXPECT_EQ(tree.allData().last(), 5);
  tree.setLeafValue(4, 0);
  EXPECT_EQ(tree.top(), 10);
  tree.setLeafValue(1, 0);
  EXPECT_EQ(tree.top(), 8);
  EXPECT_EQ(1, tree.getLeafValue(0));
  EXPECT_EQ(8, tree.getNodeValue(0));

  std::cout << "All data: " << tree.allData() << std::endl;
}

TEST(ReduceTreeTest, FindItAndIntegrate) {
  ReduceTree<double> tree([](double a, double b) {return a + b;},
      Array<double>{10, 3, 9});
  EXPECT_EQ(0, tree.findLeafIndex(5));
  EXPECT_EQ(0, tree.findLeafIndex(9.9));
  EXPECT_EQ(1, tree.findLeafIndex(10.01));
  EXPECT_EQ(1, tree.findLeafIndex(12.9));
  EXPECT_EQ(2, tree.findLeafIndex(13.1));
  EXPECT_EQ(2, tree.findLeafIndex(13.1));
  EXPECT_EQ(2, tree.findLeafIndex(21.9));
  EXPECT_EQ(-1, tree.findLeafIndex(23));

  EXPECT_NEAR(0.0, tree.integrate(-1), 1.0e-6);
  EXPECT_NEAR(0.0, tree.integrate(0), 1.0e-6);
  EXPECT_NEAR(10.0, tree.integrate(1), 1.0e-6);
  EXPECT_NEAR(13.0, tree.integrate(2), 1.0e-6);
  EXPECT_NEAR(22.0, tree.integrate(3), 1.0e-6);
  EXPECT_NEAR(22.0, tree.integrate(4), 1.0e-6);
}

