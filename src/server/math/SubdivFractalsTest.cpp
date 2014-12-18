/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/SubdivFractals.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(SubdivFractalsTest, IndexBox) {
  EXPECT_EQ(IndexBox<2>().numel(), 0);
  IndexBox<2> rulle = IndexBox<1>(2) + IndexBox<1>(3);
  EXPECT_EQ(rulle.numel(), 2*3);
  auto a = rulle.slice(0, 1);
  auto b = rulle.slice(1, 1);
  EXPECT_EQ(a.numel(), 3);
  EXPECT_EQ(b.numel(), 2);
  EXPECT_EQ(rulle.slice(1, 0, 2).numel(), 2*2);
  int inds[2] = {1, 1};
  EXPECT_EQ(rulle.calcIndex(inds), 1 + 1*2);
  EXPECT_FALSE(rulle.hasMidpoint());
}

TEST(SubdivFractalsTest, IndexBoxMidpoint) {
  EXPECT_EQ(IndexBox<2>().numel(), 0);
  IndexBox<2> rulle = IndexBox<1>(3) + IndexBox<1>(5);
  EXPECT_TRUE(rulle.hasMidpoint());
  EXPECT_EQ(rulle.midpointIndex(), 7);
  EXPECT_EQ(rulle.lowIndex(), 0);
  EXPECT_EQ(rulle.highIndex(), 14);
}
