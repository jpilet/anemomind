/*
 * PointQuadTest.cpp
 *
 *  Created on: Apr 15, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/math/PointQuad.h>

using namespace sail;

typedef PointQuad<double, 2> PQ2;

TEST(PointQuadTest, TestMeanAndVariance) {
  Eigen::Vector2d a(3, 1);
  Eigen::Vector2d b(1, 3);

  PQ2 ga(a);
  PQ2 gb(b);
  auto gc = ga + gb;
  EXPECT_NEAR(gc.computeVariance(), 2.0, 1.0e-9);
}

TEST(PointQuadTest, TestInc) {
  PQ2 x(Eigen::Vector2d(2, 3));

  x += Eigen::Vector2d(2, 9);

  auto mean = x.computeMean();

  EXPECT_NEAR(mean(0), 2, 1.0e-6);
  EXPECT_NEAR(mean(1), 6, 1.0e-6);
}


