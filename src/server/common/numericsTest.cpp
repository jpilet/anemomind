/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/common/numerics.h>
#include <limits>
#include <gtest/gtest.h>

namespace sail {

class MyType {
 public:
  MyType(double x) : _x(x) {}

  operator double() const {return _x;}
 private:
  double _x;
};

class MyType2 {
 public:
  MyType2(double x) : _x(x) {}

  double getWrappedValue() const {
    return _x;
  }
 private:
  double _x;
};

bool isFiniteMyType2(MyType2 x) {
  return std::isfinite(x.getWrappedValue());
}

bool isNaNMyType2(MyType2 x) {
  return std::isnan(x.getWrappedValue());
}

WRAP_NUMERIC_FUNCTION_FOR_NON_TEMPLATE_TYPE(IsFinite, MyType, std::isfinite)
WRAP_NUMERIC_FUNCTION_FOR_NON_TEMPLATE_TYPE(IsNaN, MyType, std::isnan)
WRAP_NUMERIC_FUNCTION_FOR_NON_TEMPLATE_TYPE(IsFinite, MyType2, isFiniteMyType2)
WRAP_NUMERIC_FUNCTION_FOR_NON_TEMPLATE_TYPE(IsNaN, MyType2, isNaNMyType2)

}

TEST(NumericTest, Numbers) {
  using namespace sail;

  std::vector<double> testNumbers{-1.0, 119.0,
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::infinity()};
  for (auto x: testNumbers) {
    std::cout << "x : " << x << std::endl;

    bool isActuallyNaN = std::isnan(x);
    bool isActuallyFinite = std::isfinite(x);

    EXPECT_EQ(isActuallyFinite, isFinite(x));
    EXPECT_EQ(isActuallyNaN, isNaN(x));
    EXPECT_EQ(isActuallyFinite, isFinite(MyType(x)));
    EXPECT_EQ(isActuallyNaN, isNaN(MyType(x)));
    EXPECT_EQ(isActuallyFinite, isFinite(MyType2(x)));
    EXPECT_EQ(isActuallyNaN, isNaN(MyType2(x)));

  }
}


