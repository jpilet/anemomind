/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/nautical/calibration/LinearFit.h>

using namespace sail;

namespace {
  Eigen::MatrixXd scalarToMat(double x) {
    Eigen::MatrixXd X(1, 1);
    X(0, 0) = x;
    return X;
  }
}

TEST(LinearFitTest, BuildEqs) {
  auto a = LinearFit::buildNormalEqs(Angle<double>::degrees(0.0), scalarToMat(0.0));
  auto b = LinearFit::buildNormalEqs(Angle<double>::degrees(45.0), scalarToMat(2.3));
  auto c = LinearFit::buildNormalEqs(Angle<double>::degrees(90.0), scalarToMat(4.6));
  auto abc = a + b + c;
  auto P = abc.solve();

  double expected[3] = {-2.3, 2.3, 2.3};
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(expected[i], P(i, 0), 1.0e-9);
  }
}

TEST(LinearFitTest, BuildEqs2) {
  auto a = LinearFit::buildNormalEqs(Angle<double>::degrees(0.0), scalarToMat(1.0));
  auto b = LinearFit::buildNormalEqs(Angle<double>::degrees(45.0), scalarToMat(-3.0));
  auto c = LinearFit::buildNormalEqs(Angle<double>::degrees(90.0), scalarToMat(-7.0));
  auto abc = a + b + c;
  auto P = abc.solve();

  double expected[3] = {4.0, -4.0, -3.0};
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(expected[i], P(i, 0), 1.0e-9);
  }
}

TEST(LinearFitTest, BasicTest) {
  NavalSimulation sim = getNavSimFractalWindOriented();
  auto boatData = sim.boatData(0);
}


