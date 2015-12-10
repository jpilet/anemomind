/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/nautical/calibration/LinearFit.h>
#include <server/nautical/calibration/LinearCalibration.h>

using namespace sail;

namespace {
  Eigen::MatrixXd scalarToMat(double x) {
    Eigen::MatrixXd X(1, 1);
    X(0, 0) = x;
    return X;
  }

  Eigen::MatrixXd matRow(double x, double y) {
    Eigen::MatrixXd X(1, 2);
    X(0, 0) = x;
    X(0, 1) = y;
    return X;
  }
}

TEST(LinearFitTest, BuildEqs) {
  auto a = LinearFit::buildNormalEqs(Angle<double>::degrees(0.0), scalarToMat(0.0));
  auto b = LinearFit::buildNormalEqs(Angle<double>::degrees(45.0), scalarToMat(2.3));
  auto c = LinearFit::buildNormalEqs(Angle<double>::degrees(90.0), scalarToMat(4.6));
  auto abc = a + b + c;
  auto P = abc.luSolve();

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
  auto P = abc.luSolve();

  double expected[3] = {4.0, -4.0, -3.0};
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(expected[i], P(i, 0), 1.0e-9);
  }
}

TEST(LinearFitTest, CoefMats) {
  auto a = LinearFit::buildNormalEqs(Angle<double>::degrees(0.0), matRow(0.0, 1.0));
  auto b = LinearFit::buildNormalEqs(Angle<double>::degrees(45.0), matRow(2.3, -3.0));
  auto c = LinearFit::buildNormalEqs(Angle<double>::degrees(90.0), matRow(4.6, -7.0));
  auto X = a + b + c;

  auto d = LinearFit::buildNormalEqs(Angle<double>::degrees(0.0), matRow( 1.0, 0.0));
  auto e = LinearFit::buildNormalEqs(Angle<double>::degrees(45.0), matRow(-3.0, 2.3));
  auto f = LinearFit::buildNormalEqs(Angle<double>::degrees(90.0), matRow(-7.0, 4.6));
  auto Y = d + e + f;

  auto coefs = LinearFit::makeXYCoefMatrices(X, Y);

  EXPECT_EQ(coefs.A.rows(), 4);
  EXPECT_EQ(coefs.A.cols(), 1);
  EXPECT_EQ(coefs.B.rows(), 4);
  EXPECT_EQ(coefs.B.cols(), 1);
  auto tol = 1.0e-9;
  EXPECT_NEAR(coefs.A(0, 0), -2.3, tol);
  EXPECT_NEAR(coefs.A(1, 0), 2.3, tol);
  EXPECT_NEAR(coefs.B(0, 0), 4.0, tol);
  EXPECT_NEAR(coefs.B(1, 0), -4.0, tol);

  EXPECT_NEAR(coefs.A(2, 0), 4.0, tol);
  EXPECT_NEAR(coefs.A(3, 0), -4.0, tol);
  EXPECT_NEAR(coefs.B(2, 0), -2.3, tol);
  EXPECT_NEAR(coefs.B(3, 0), 2.3, tol);
}

TEST(LinearFitTest, BasicTest) {
  using namespace LinearCalibration;
  using namespace LinearFit;
  using namespace EigenUtils;
  NavalSimulation sim = getNavSimFractalWindOriented();
  auto boatData = sim.boatData(0);

  LinearCalibration::FlowSettings settings;
  auto navs = boatData.navs();
  auto headings = getMagHdg(navs);
  auto flow = LinearCalibration::makeTrueWindMatrices(navs, settings);
  auto Xeqs = LinearFit::makeNormalEqs(headings, flow, 0);
  auto Yeqs = LinearFit::makeNormalEqs(headings, flow, 1);
  int n = navs.size();
  auto spans = makeOverlappingSpans(n, 100, 0.5);

  auto coefs = makeCoefMatrices(Xeqs, Yeqs, spans);
  std::cout << EXPR_AND_VAL_AS_STRING(minimizeLeastSquares(coefs)) << std::endl;

}


