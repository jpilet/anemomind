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
  typedef Polynomial<double, 1> P1;
  auto p = P1{3} + P1{4};
  EXPECT_EQ(p, P1{7});
}


