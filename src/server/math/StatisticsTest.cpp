/*
 *  Created on: 2014-07-02
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/Statistics.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(StatisticsTest, BasicTest) {
  Statistics s;
  EXPECT_FALSE(s.defined());
  s.add(1.0);
  EXPECT_TRUE(s.defined());
  EXPECT_NEAR(s.mean(), 1.0, 1.0e-9);
  EXPECT_NEAR(s.stddev(), 0.0, 1.0e-9);
  s.add(1.0);
  EXPECT_NEAR(s.mean(), 1.0, 1.0e-9);
  EXPECT_NEAR(s.stddev(), 0.0, 1.0e-9);
  s.add(4.0);
  EXPECT_NEAR(s.mean(), 2.0, 1.0e-9);
  EXPECT_NEAR(s.mean(), 2.0, 1.0e-9);
  EXPECT_NEAR(s.variance(), 2.0, 1.0e-9); // (1² + 1² + 4²)/3 - 2² = 18/3 - 4 = 6 - 4 = 2
  EXPECT_EQ(s.count(), 3);
}


