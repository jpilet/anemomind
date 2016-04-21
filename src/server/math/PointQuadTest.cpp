/*
 * PointQuadTest.cpp
 *
 *  Created on: Apr 15, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/math/PointQuad.h>

TEST(ExperimentalTest, PointQuad) {
  typedef PointQuad<double, 2> Gd;

  Eigen::Vector2d a(3, 1);
  Eigen::Vector2d b(1, 3);

  Gd ga(a);
  Gd gb(b);
  auto gc = ga + gb;
  EXPECT_NEAR(gc.computeVariance(gc.computeMean()), 2.0, 1.0e-9);
}


