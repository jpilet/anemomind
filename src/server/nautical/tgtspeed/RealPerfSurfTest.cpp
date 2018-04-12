/*
 * RealPerfSurfTest.cpp
 *
 *  Created on: 2 Mar 2018
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/tgtspeed/RealPerfSurf.h>

using namespace sail;

TEST(RealPerfSurfTest, PriorTest) {
  EXPECT_NEAR(0.0, twaPrior(0.0_deg), 1.0e-6);
  EXPECT_NEAR(1.0, twaPrior(90.0_deg), 0.1);
  EXPECT_NEAR(1.0, twaPrior(180.0_deg), 1.0e-9);
  EXPECT_NEAR(1.0, twaPrior(540.0_deg), 1.0e-9);
}

TEST(RealPerfSurfTest, RegTest) {
  auto R = makeFirstOrderReg(2, {{0, 1}});
  EXPECT_EQ(R.rows(), 1);
  EXPECT_EQ(R.cols(), 2);
  EXPECT_EQ(R(0, 0), 1.0);
  EXPECT_EQ(R(0, 1), -1.0);
}
