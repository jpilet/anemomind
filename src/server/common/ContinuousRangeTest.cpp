#include <gtest/gtest.h>
#include "ContinuousRange.h"
#include <cmath>

using namespace sail;

TEST(ContinuousRangeTest, TestIntersects) {
  EXPECT_TRUE(ContinuousRange(Arrayd::args(1, 2), false).intersects(
                ContinuousRange(Arrayd::args(1.5, 3), false)));
  EXPECT_FALSE(ContinuousRange(Arrayd::args(1, 2), false).intersects(
                 ContinuousRange(Arrayd::args(3, 4), false)));
  EXPECT_TRUE(ContinuousRange(Arrayd::args(0.0, 0.3), true).intersects(
                ContinuousRange(Arrayd::args(0.1
                                + 4.0*M_PI, 0.3), true)));
  EXPECT_FALSE(ContinuousRange(Arrayd::args(0.0, 0.3), true).intersects(
                 ContinuousRange(Arrayd::args(0.4
                                 + 4.0*M_PI, 0.31), true)));
  EXPECT_TRUE(ContinuousRange(Arrayd::args(0.0, 0.3), true).intersects(
                ContinuousRange(Arrayd::args(0.0, 0.3), true)));
}
