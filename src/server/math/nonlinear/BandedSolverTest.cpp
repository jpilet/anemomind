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
  AbsCost regCost;

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

/*
 * This example shows that, if we use AbsCost for both the data and the regularization,
 * the filtered signal will have the same shape, even if we scale both the underlying true
 * signal and the noise. That would probably not be the case if we mix for instance square
 * and abs.
 *
 * This is useful, because if the signal-to-noise-ratio is constant, it doesn't matter
 * what unit we use. For instance, if we filter the boat speed, it doesn't matter if
 * we perform our calculations in knots, m/s or km/h.
 *
 * Also, the abs cost is less sensitive to outliers.
 */
TEST(BandedSolver, RampMultiscale) {
  bool visualize = false;

  int scaleCount = 4;
  LineKM scaleMap(0, scaleCount-1, log(1.0), log(120.0));


  AbsCost regCost, dataCost;

  // 30 samples ranging from -1 to 1.
  Sampling sampling(30, -1, 1);

  MDArray2d Yprevious;
  double previousScale = 1.0;

  for (int i = 0; i < scaleCount; i++) {
    double scale = exp(scaleMap(i));
    int obsCount = 300;
    Array<Observation<1> > obs(obsCount);
    Arrayd obsX(obsCount), obsY(obsCount);
    for (int i = 0; i < obsCount; i++) {
      auto x = -1 + 2.0*positiveMod(sin(234.234*i) + sin(233.8*i), 1.0);
      auto noise = 0.1*sin(3443.0*i);
      auto y = scale*(theSignal(x) + noise);
      obs[i] = Observation<1>{sampling.represent(x), y};
      obsX[i] = x;
      obsY[i] = y;
    }

    BandedSolver::Settings s;
    s.regOrder = 2;
    s.lambda = 4;
    s.iters = 30;
    MDArray2d Y = BandedSolver::solve(dataCost, regCost, sampling, obs, s);

    for (int i = 0; i < sampling.count(); i++) {
      double x = sampling.indexToX()(i);
      EXPECT_NEAR(Y(i, 0), scale*theSignal(x), scale*0.1);
    }

    // Check that it is close to a scaled version of the previous signal.
    if (!Yprevious.empty()) {
      double f = scale/previousScale;
      for (int i = 0; i < sampling.count(); i++) {
        EXPECT_NEAR(f*Yprevious(i, 0), Y(i, 0), 0.1*scale);
      }
    }

    Yprevious = Y;
    previousScale = scale;

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
}


