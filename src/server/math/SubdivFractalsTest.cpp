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
}


