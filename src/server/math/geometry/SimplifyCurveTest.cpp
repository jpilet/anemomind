#include <server/math/geometry/SimplifyCurve.h>

#include <gtest/gtest.h>

TEST(SimplifyCurveTest, SimpleOpen) {
  CurveSimplifier curve(false);

  // the 3 points are almost aligned. The middle one should be the first
  // eliminated.
  curve.addPoint(0, 0);
  curve.addPoint(1, .55);
  curve.addPoint(2, 1);

  std::vector<int> priority = curve.priorities();

  EXPECT_EQ(3, priority.size());
  EXPECT_EQ(0, priority[0]);
  EXPECT_EQ(2, priority[1]);
  EXPECT_EQ(1, priority[2]);
}

TEST(SimplifyCurveTest, SimpleClosed) {
  CurveSimplifier curve(true);

  curve.addPoint(0, 0);
  curve.addPoint(1, 0);
  curve.addPoint(.1, .5);
  curve.addPoint(0, .5);

  std::vector<int> priority = curve.priorities();

  EXPECT_EQ(4, priority.size());
  EXPECT_EQ(2, priority[0]);
  EXPECT_EQ(3, priority[2]);
}

TEST(SimplifyCurveTest, ColinearClosed) {
  CurveSimplifier curve(true);
  curve.addPoint(0, 0);
  curve.addPoint(1, 2);
  curve.addPoint(2, 4);
  std::vector<int> priorities = curve.priorities();
  EXPECT_EQ(3, priorities.size());
  EXPECT_EQ(2, priorities[1]);
}
