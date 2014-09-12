/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/CumulativeFunction.h>

using namespace sail;

TEST(CumulativeFunctionTest, BasicTest) {
  Arrayd X(2);
  X[0] = 1.0;
  X[1] = 2.0;
  CumulativeFunction cfun(X);
  EXPECT_NEAR(cfun.eval(0.0), 0.0, 1.0e-9);
  EXPECT_NEAR(cfun.eval(0.5), 0.5, 1.0e-9);
  EXPECT_NEAR(cfun.eval(1.0), 1.0, 1.0e-9);
}


