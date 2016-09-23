/*
 * PositiveModTest.cpp
 *
 *  Created on: Jun 9, 2016
 *      Author: jonas
 */

#include <iostream>
#include <server/common/PositiveMod.h>
#include <cmath>
#include <gtest/gtest.h>
#include <ceres/jet.h>

using namespace sail;

namespace {
  void testMakePositiveCyclic(int a, int b) {
    auto positive = makePositiveCyclic(a, b);
    EXPECT_LE(0, positive);
    EXPECT_EQ(positive % b, (a + 3000*b) % b);
  }

  void testPositiveMod(int a, int b) {
    auto c = positiveMod(a, b);
    EXPECT_LE(0, c);
    EXPECT_LT(c, b);
    EXPECT_EQ(c, (a + 3000*b) % b);
  }

  void testItThoroughly(int a, int b) {
    testMakePositiveCyclic(a, b);
    testPositiveMod(a, b);
  }
}

TEST(PositiveModTest, makePositiveCyclic) {
  for (int i = -30; i < 30; i++) {
    testItThoroughly(i, 4);
    testItThoroughly(i, 5);
    testItThoroughly(i, 6);
  }
}


TEST(PositiveModTest, HugeNumber) {
  double v = std::numeric_limits<double>::max();
  double y = positiveMod(v, 2.0);
  EXPECT_LE(0.0, y);
  EXPECT_LT(y, 2.0);
}

TEST(PositiveModTest, HugeNegativeNumber) {
  double v = std::numeric_limits<double>::min();
  double y = positiveMod(v, 2.0);
  EXPECT_LE(0.0, y);
  EXPECT_LT(y, 2.0);
}

namespace {
  class MinimalistNumber {
  public:
    MinimalistNumber(int x) : _x(x) {}

    MinimalistNumber operator+(MinimalistNumber y) {
      return MinimalistNumber(_x + y._x);
    }

    MinimalistNumber operator-(MinimalistNumber y) {
      return MinimalistNumber(_x - y._x);
    }

    bool operator<(MinimalistNumber y) const {
      return _x < y._x;
    }

    int unwrap() const {return _x;}
  private:
    int _x;
  };
}

TEST(PositiveModTest, WithSpecialType) {
  EXPECT_EQ(3, positiveMod(MinimalistNumber(-1), MinimalistNumber(4)).unwrap());
}

TEST(PositiveModTest, WithCeres) {
  typedef ceres::Jet<double, 1> AD;
  EXPECT_EQ(3.0, positiveMod(AD(-1.0), AD(4.0)).a);
}

TEST(PositiveModTest, NaNValue) {
  EXPECT_TRUE(std::isnan(positiveMod(std::numeric_limits<double>::quiet_NaN(), 2.0)));
}

TEST(PositiveModTest, InfValue) {
  EXPECT_FALSE(std::isfinite(positiveMod(std::numeric_limits<double>::infinity(), 2.0)));
  EXPECT_FALSE(std::isfinite(positiveMod(-std::numeric_limits<double>::infinity(), 2.0)));
}

