/*
 * AxisTicksTest.cpp
 *
 *  Created on: 23 Oct 2016
 *      Author: jonas
 */
#include <gtest/gtest.h>
#include <server/plot/AxisTicks.h>

using namespace sail;

TEST(AxisTicksTest, TestBasicIterator) {
  BasicTickIterator iter(0, "meters");
  EXPECT_NEAR(iter.tickSpacing(), 1.0, 1.0e-6);
  auto x = iter.get(4);
  EXPECT_NEAR(x.position, 4.0, 1.0e-6);
  EXPECT_EQ(x.tickLabel, "4 meters");
}
