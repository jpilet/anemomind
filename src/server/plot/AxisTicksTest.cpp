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
  EXPECT_NEAR(iter.coarser().tickSpacing(), 10.0, 1.0e-6);
  EXPECT_NEAR(iter.finer().tickSpacing(), 0.1, 1.0e-6);
  auto k = iter.coarser();
  EXPECT_EQ(k.get(4).tickLabel, "40 meters");
  EXPECT_NEAR(k.computeFracIndex(33), 3.3, 1.0e-6);
}

TEST(AxisTicksTest, DateTest) {
  DateTickIterator iter(0);
  {
    auto x = iter.get(2016*12);
    EXPECT_EQ(x.tickLabel, "January 2016");
  }{
    auto x = iter.get(2016*12 + 1);
    EXPECT_EQ(x.tickLabel, "February 2016");
  }
  auto index = iter.computeFracIndex(
      TimeStamp::UTC(2014, 1, 1, 0, 0, 0.0));
  EXPECT_NEAR(index, 2014*12, 1.0e-6);
}
