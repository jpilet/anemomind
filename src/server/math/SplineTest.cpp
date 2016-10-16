/*
 * SplineTest.cpp
 *
 *  Created on: 12 Oct 2016
 *      Author: jonas
 */

#include <server/math/Spline.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(SplineBasisTest, TestIt) {
  SplineBasis<double, 1> s1;
  s1.pieces[0] = Polynomial<double, 1>(1.0);
  EXPECT_EQ(s1.boundary(0), -0.5);
  EXPECT_EQ(s1.boundary(1), 0.5);

  EXPECT_EQ(s1.pieceIndex(-0.6), -1);
  EXPECT_EQ(s1.pieceIndex(-0.4), 0);
  EXPECT_EQ(s1.pieceIndex(0.6), 1);

  EXPECT_EQ(s1(-0.6), 0.0);
  EXPECT_EQ(s1(-0.4), 1.0);
  EXPECT_EQ(s1(0.4), 1.0);
  EXPECT_EQ(s1(0.6), 0.0);

  auto s2 = s1.next();
  EXPECT_NEAR(s2(-1.0), 0.0, 1.0e-6);
  EXPECT_NEAR(s2(-0.5), 0.5, 1.0e-6);
  EXPECT_NEAR(s2(0.0), 1.0, 1.0e-6);
  EXPECT_NEAR(s2(0.5), 0.5, 1.0e-6);
  EXPECT_NEAR(s2(1.0), 0.0, 1.0e-6);
}


