/*
 * Polynomial.cpp
 *
 *  Created on: 16 Oct 2016
 *      Author: jonas
 */

#include <server/math/Polynomial.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(SplineTest, PolyTest) {
  typedef Polynomial<double, 1> P1; // constant
  typedef Polynomial<double, 2> P2; // straight line
  typedef Polynomial<double, 3> P3; // second-order polynomial
  auto p = P1{3} + P1{4};
  EXPECT_EQ(p, P1{7});
  EXPECT_FALSE(P1{8} == P1{7});
  EXPECT_EQ(P1{5} - P1{3}, P1{2});
  EXPECT_EQ((P1{4} + P2{0, 5}), (P2{4, 5}));

  EXPECT_EQ((P1{4}.primitive()), (P2{0, 4}));
  EXPECT_EQ((P2{0, 4}.primitive()), (P3{0, 0, 2}));

  EXPECT_EQ((P2{3, 4}*P2{4, 5}), (P3{12, 31, 20}));

  EXPECT_EQ(eval(P1{3}, P1{4}), (P1{3}));
  EXPECT_EQ(eval(P2{3, 9}, P1{7}), (P1{66}));
  EXPECT_EQ(eval(P3{0, 0, 1}, P2{5, 4}), (P3{25, 40, 16}));
}

TEST(SplineTest, Derivative) {
  auto p2 = Polynomial<double, 3>{3.0, 4.0, 5.0};
  auto p1 = Polynomial<double, 2>{4.0, 10.0};
  auto p0 = Polynomial<double, 1>{10.0};

  EXPECT_EQ(p1, p2.derivative());
  EXPECT_EQ(p0, p1.derivative());
}
