/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nonlinear/BandedSolver.h>
#include <gtest/gtest.h>
#include <server/plot/extra.h>
#include <iostream>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>

using namespace sail;

TEST(BandedSolver, TVFilterFirstOrder) {
  SquareCost dataCost;
  AbsCost regCost(0.0001);

  // 30 samples ranging from -1 to 1.
  Sampling sampling(30, -1, 1);

  int obsCount = 300;
  Array<Observation<1> > obs(obsCount);
  Arrayd obsX(obsCount), obsY(obsCount);
  for (int i = 0; i < obsCount; i++) {
    auto x = -1 + 2.0*positiveMod(sin(234.234*i) + sin(233.8*i), 1.0);
    auto y = (x < 0.0? -1.0 : 1.0) + 0.1*sin(3443.0*i);
    obs[i] = Observation<1>{sampling.represent(x), y};

    obsX[i] = x;
    obsY[i] = y;
  }

  BandedSolver::Settings s;
  s.lambda = 1;
  s.iters = 4;
  MDArray2d Y = BandedSolver::solve(dataCost, regCost, sampling, obs, s);
  for (int i = 0; i < sampling.count(); i++) {
    double x = sampling.indexToX()(i);
    EXPECT_NEAR(Y(i, 0), (x < 0? -1 : 1), 0.04);
  }

  bool visualize = false;

  if (visualize) {
    GnuplotExtra plot;
    plot.set_style("lines");
    Arrayd X = sampling.makeX();
    plot.plot_xy(X, Y.getStorage());
    plot.set_style("points");
    plot.plot_xy(obsX, obsY);
    plot.show();

    std::cout << EXPR_AND_VAL_AS_STRING(Y) << std::endl;
  }
}

double theSignal(double x) {
  double a = 0.2;
  if (x < -a) {
    return -1.0;
  } else if (x < a) {
    return LineKM(-a, a, -1, 1)(x);
  }
  return 1.0;
}

TEST(BandedSolver, Ramp) {
  AbsCost regCost(0.0001), dataCost(0.0001);

  // 30 samples ranging from -1 to 1.
  Sampling sampling(30, -1, 1);

  int obsCount = 300;
  Array<Observation<1> > obs(obsCount);
  Arrayd obsX(obsCount), obsY(obsCount);
  for (int i = 0; i < obsCount; i++) {
    auto x = -1 + 2.0*positiveMod(sin(234.234*i) + sin(233.8*i), 1.0);
    auto y = theSignal(x) + 0.1*sin(3443.0*i);
    obs[i] = Observation<1>{sampling.represent(x), y};
    obsX[i] = x;
    obsY[i] = y;
  }

  BandedSolver::Settings s;
  s.regOrder = 2;
  s.lambda = 1.0;
  MDArray2d Y = BandedSolver::solve(dataCost, regCost, sampling, obs, s);

  for (int i = 0; i < sampling.count(); i++) {
    double x = sampling.indexToX()(i);
    EXPECT_NEAR(Y(i, 0), theSignal(x), 0.04);
  }

  bool visualize = false;

  if (visualize) {
    GnuplotExtra plot;
    plot.set_style("lines");
    Arrayd X = sampling.makeX();
    plot.plot_xy(X, Y.getStorage());
    plot.set_style("points");
    plot.plot_xy(obsX, obsY);
    plot.show();

    std::cout << EXPR_AND_VAL_AS_STRING(Y) << std::endl;
  }
}


