/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nonlinear/SignalUtils.h>
#include <gtest/gtest.h>

using namespace sail;

double numericDerivative(const RobustCost &c, double x, double s) {
  constexpr double h = 1.0e-4;
  return s*(c.eval(s*(x + h)) - c.eval(s*(x - h)))/(2.0*h);
}

TEST(RobustSignal, CostFun) {
  {
    RobustCost cost(1.0);
    auto large = 1.0e9;
    EXPECT_NEAR(cost.eval(large), 0.0, 1.0e-6);
    EXPECT_LE(cost.eval(large), 0);
    EXPECT_LT(cost.eval(1000), 0.0);
    EXPECT_NEAR(cost.eval(0), -1, 1.0e-6);
  }{
    RobustCost cost(1.0);
    EXPECT_NEAR(evalScaled(cost, 0, 2.0), -2, 1.0e-6);
  }{
    RobustCost cost(2.0);
    EXPECT_NEAR(evalScaled(cost, 0, 1.0), -4, 1.0e-6);
    EXPECT_NEAR(cost.evalDerivative(1.2), numericDerivative(cost, 1.2, 1.0), 1.0e-3);
  }{
    double x = 1.2;
    auto s = 1.3;
    RobustCost cost(3.9);
    EXPECT_NEAR(evalDerivativeScaled(cost, x, s), numericDerivative(cost, x, s), 1.0e-3);
    EXPECT_NEAR(evalDerivativeScaled(cost, x, s), -evalDerivativeScaled(cost, -x, s), 1.0e-3);
    auto maj = majorizeCostFunction(cost, x);
    EXPECT_NEAR(maj.b, 0.0, 1.0e-6);
    auto majDeriv = 2.0*maj.a*x;
    EXPECT_NEAR(majDeriv, cost.evalDerivative(x), 1.0e-6);

    double refDif = maj.eval(x) - cost.eval(x);

    for (double y = -9.0; y < 9; y += 0.1) {
      EXPECT_LE(refDif, maj.eval(y) - cost.eval(y));
    }
  }
}



