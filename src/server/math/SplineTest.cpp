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

  /*EXPECT_EQ(s1.eval(-0.6), 0.0);
  EXPECT_EQ(s1.eval(-0.4), 1.0);
  EXPECT_EQ(s1.eval(0.4), 1.0);
  EXPECT_EQ(s1.eval(0.6), 0.0);*/
}


