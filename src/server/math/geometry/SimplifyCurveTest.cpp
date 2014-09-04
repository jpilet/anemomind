#include <server/math/geometry/SimplifyCurve.h>

#include <gtest/gtest.h>

TEST(SimplifyCurveTest, SimpleOpen) {
  Curve curve(false);

  // the 3 points are almost aligned. The middle one should be the first
  // eliminated.
  curve.addPoint(0, 0);
  curve.addPoint(1, .55);
  curve.addPoint(2, 1);

  std::vector<int> priority = curve.priorities();

  EXPECT_EQ(0, priority[0]);
  EXPECT_EQ(2, priority[1]);
  EXPECT_EQ(1, priority[2]);
}

TEST(SimplifyCurveTest, SimpleClosed) {
  Curve curve(true);

  curve.addPoint(0, 0);
  curve.addPoint(1, 0);
  curve.addPoint(.1, .5);
  curve.addPoint(0, .5);

  std::vector<int> priority = curve.priorities();

  EXPECT_EQ(1, priority[0]);
  EXPECT_EQ(0, priority[1]);
  EXPECT_EQ(3, priority[2]);
  EXPECT_EQ(2, priority[3]);
}
