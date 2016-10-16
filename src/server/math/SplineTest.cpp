/*
 * SplineTest.cpp
 *
 *  Created on: 12 Oct 2016
 *      Author: jonas
 */

#include <server/math/Spline.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(SplineTest, PolyTest) {
  typedef Polynomial<double, 1> P1; // constant
  typedef Polynomial<double, 2> P2; // straight line
  typedef Polynomial<double, 3> P3; // second-order polynomial
  auto p = P1{3} + P1{4};
  EXPECT_EQ(p, P1{7});
  EXPECT_FALSE(P1{8} == P1{7});

  EXPECT_EQ((P1{4}.primitive()), (P2{0, 4}));
}


