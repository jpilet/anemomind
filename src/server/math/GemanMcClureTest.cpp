/*
 *  Created on: 2014-04-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <server/math/nonlinear/Levmar.h>
#include <server/math/nonlinear/LevmarSettings.h>
#include <gtest/gtest.h>
#include <server/math/GemanMcClure.h>
#include <iostream>
#include <server/common/Uniform.h>
#include <server/common/SharedPtrUtils.h>
#include <server/math/ADFunction.h>
#include <server/common/string.h>
#include <server/common/ScopedLog.h>

using namespace sail;

TEST(GemanMcClureTest, WeightTest) {
  GemanMcClure gmc(10.0, 10.0, 1, 1);
  EXPECT_NEAR(gmc.getResidualWeight(0), 0.5, 1.0e-6);
  gmc.addResiduals(Arrayd{10.0});
  EXPECT_NEAR(gmc.getResidualWeight(0), 0.5, 1.0e-6);
  gmc.addResiduals(Arrayd{10000.1});
  EXPECT_LE(gmc.getResidualWeight(0), 0.4);
}

TEST(GemanMcClureTest, OutlierCostTestHighInitial) {
  GemanMcClure gmc(10.0, 1.0e9, 2, 1);
  EXPECT_NEAR(gmc.calcOutlierCost(), 10*10 + 10*10, 1.0e-4);
  EXPECT_NEAR(gmc.calcOutlierResidual(), sqrt(10*10 + 10*10), 1.0e-4);
}

TEST(GemanMcClureTest, OutlierCostTestLowInitial) {
  GemanMcClure gmc(10.0, 0.0, 2, 1);
  EXPECT_NEAR(gmc.calcOutlierCost(), 0.0, 1.0e-6);
}

TEST(GemanMcClureTest, Change) {
  GemanMcClure gmc(10.0, 0.0, 1, 1);
  double initW = gmc.getResidualWeight(0);
  double initOc = gmc.calcOutlierCost();
  gmc.addResiduals(Arrayd{1.0});
  EXPECT_LE(gmc.getResidualWeight(0), initW);
  EXPECT_LE(initOc, gmc.calcOutlierCost());
}


TEST(GemanMcClureTest, Cost) {
  GemanMcClure gmc(1.0, 0.0, 1, 1);
  EXPECT_NEAR(gmc.calcCost(1.0),
      sqr(0.5)*sqr(1.0) + sqr(1.0 - 0.5)*sqr(1.0), 1.0e-6);
}

TEST(GemanMcClureTest, Cost2) {
  double sigma = 10.0;
  GemanMcClure gmc(sigma, 0.0, 0.0, 1);
  EXPECT_NEAR(gmc.calcCost(sigma),
      sqr(0.5)*sqr(sigma) + sqr(1.0 - 0.5)*sqr(sigma),
      1.0e-6);
}

namespace {
const double a = 0.38;
const double b = 4.2;
const double c = 1.09;

  double polyfun(double x) {
    return a + x*(b + c*x);
  }

  class PolyFit : public AutoDiffFunction {
   public:
    PolyFit(Arrayd X, Arrayd Y) : _X(X), _Y(Y) {
      assert(X.size() == Y.size());
    }
    int inDims() {return 3;}
    int outDims() {return _Y.size();}
    void evalAD(adouble *Xin, adouble *Fout);
   private:
    Arrayd _X, _Y;
  };

  void PolyFit::evalAD(adouble *Xin, adouble *Fout) {
    int n = outDims();
    for (int i = 0; i < n; i++) {
      double x = _X[i];
      double y = _Y[i];
      Fout[i] = Xin[0] + x*(Xin[1] + x*Xin[2]) - y;
    }
  }
}

TEST(GemanMcClureTest, Modelfitting) {
  Uniform::initialize(1398349409);
  ScopedLog::setDepthLimit(0);

  int count = 30;
  Uniform xgen(-1.0, 1.0);
  double k = 0.01;

  // Choose a value that is high enough to
  // classify inliers as inliers,
  // but not so high that we classify inliers
  // as outliers.
  double sigma = 3*k;

  double probOutlier = 0.2;

  Uniform noise(-k, k);
  Uniform p01(0.0, 1.0);
  Uniform outlierGen(-30, 30);

  Arrayd X(count), Ygt(count), Y(count);
  Arrayb inlier(count);

  int outlierCount = 0;
  for (int i = 0; i < count; i++) {
    X[i] = xgen.gen();
    Ygt[i] = polyfun(X[i]);
    inlier[i] = p01.gen() >= probOutlier;

    if (inlier[i]) {
      Y[i] = Ygt[i] + noise.gen();
    } else {
      Y[i] = outlierGen.gen();
      outlierCount++;
    }
  }
  EXPECT_LT(0, outlierCount);

  Arrayd initCoefs = Arrayd::fill(3, 1.0);
  PolyFit fitnessNonRobust(X, Y);

  GemanMcClureFunction fitnessRobust(sigma, sigma, 1,
      makeSharedPtrToStack(fitnessNonRobust));


  LevmarSettings settings;
  LevmarState state(initCoefs);
  state.minimize(settings, fitnessRobust);
  Arrayd Xopt = state.getXArray();





  /*
   * Check correspondence of the polynomial coefficients
   */
  const double marg = 0.1;
  EXPECT_NEAR(a, Xopt[0], a*marg);
  EXPECT_NEAR(b, Xopt[1], b*marg);
  EXPECT_NEAR(c, Xopt[2], c*marg);

  /*
   * Check that all inliers are correctly classified as inliers
   * and all outliers correctly classified as outliers.
   */
  Arrayd residuals(fitnessRobust.outDims());
  fitnessNonRobust.eval(Xopt.ptr(), residuals.ptr(), nullptr);
  for (int i = 0; i < count; i++) {
    EXPECT_EQ(inlier[i], fitnessRobust.gmc().isInlier(i, residuals));
  }
}





