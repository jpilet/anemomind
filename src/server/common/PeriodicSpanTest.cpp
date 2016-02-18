#include <gtest/gtest.h>
#include "PeriodicSpan.h"
#include <cmath>

using namespace sail;

TEST(ContinuousRangeTest, TestIntersects) {
  EXPECT_TRUE(PeriodicSpan(Arrayd{0.0, 0.3}).intersects(
                PeriodicSpan(Arrayd{0.1
                                + 4.0*M_PI, 0.3})));
  EXPECT_FALSE(PeriodicSpan(Arrayd{0.0, 0.3}).intersects(
                 PeriodicSpan(Arrayd{0.4
                                 + 4.0*M_PI, 0.31})));
  EXPECT_TRUE(PeriodicSpan(Arrayd{0.0, 0.3}).intersects(
                PeriodicSpan(Arrayd{0.0, 0.3})));
}
