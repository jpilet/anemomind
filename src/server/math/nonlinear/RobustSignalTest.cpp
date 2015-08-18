/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nonlinear/RobustSignal.h>
#include <gtest/gtest.h>

using namespace sail;
using namespace sail::RobustSignal;

TEST(RobustSignal, CostFun) {
  {
    RobustCost cost(1.0, 1.0);
    auto large = 1.0e9;
    EXPECT_NEAR(cost.eval(large), 0.0, 1.0e-6);
    EXPECT_LE(cost.eval(large), 0);
    EXPECT_LT(cost.eval(1000), 0.0);
    EXPECT_NEAR(cost.eval(0), -1, 1.0e-6);
  }{
    RobustCost cost(1.0, 2.0);
    EXPECT_NEAR(cost.eval(0), -2, 1.0e-6);
  }{
    RobustCost cost(2.0, 1.0);
    EXPECT_NEAR(cost.eval(0), -4, 1.0e-6);
  }
}



