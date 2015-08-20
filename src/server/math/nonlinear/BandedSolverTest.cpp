/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nonlinear/BandedSolver.h>
#include <gtest/gtest.h>

#include <iostream>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>

using namespace sail;

TEST(BandedSolver, SolveStepL1) {
  AbsCost cost(0.0001);

  // 30 samples ranging from -1 to 1.
  Sampling sampling(30, -1, 1);

  int obsCount = 300;
  Array<Observation<1> > obs(obsCount);
  for (int i = 0; i < obsCount; i++) {
    auto x = -1 + 2.0*positiveMod(sin(234.234*i) + sin(233.8*i), 1.0);
    auto y = (x < 0.0? -1.0 : 1.0) + 0.1*sin(3443.0*i);
    obs[i] = Observation<1>{sampling.represent(x), y};
  }

  BandedSolver::Settings s;
  s.lambda = 10;
  std::cout << EXPR_AND_VAL_AS_STRING(s.lambda) << std::endl;
  MDArray2d X = BandedSolver::solve(cost, cost, sampling, obs, s);
  for (int i = 0; i < sampling.count(); i++) {
    double x = sampling.indexToX()(i);
    EXPECT_NEAR(X(i, 0), (x < 0? -1 : 1), 0.01);
  }
}


