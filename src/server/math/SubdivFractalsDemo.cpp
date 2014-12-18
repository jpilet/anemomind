/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <random>
#include <server/plot/extra.h>
#include <server/math/SubdivFractals.h>

using namespace sail;

int main() {
  std::default_random_engine e;

  int ruleCount = 12;
  MDArray2i inds(ruleCount, ruleCount);
  MDArray2d lambda(ruleCount, ruleCount);

  double marg = 0.2;
  std::uniform_real_distribution<double> lambdaDistrib(-marg, 1 + marg);
  std::uniform_int_distribution<int> indexDistrib(0, ruleCount-1);
  for (int i = 0; i < ruleCount; i++) {
    for (int j = 0; j < ruleCount; j++) {
      lambda(i, j) = lambdaDistrib(e);
      inds(i, j) = indexDistrib(e);
    }
  }

  double ctrl[4] = {0, 1, 1, 2};
  int ctrlClasses[4] = {0, 1, 2, 3};

  SubdivFractals<1> f(inds, lambda);

  int sampleCount = 3000;
  Arrayd X(sampleCount);
  Arrayd Y(sampleCount);
  for (int i = 0; i < sampleCount; i++) {
    double x = double(i)/sampleCount;
    X[i] = x;
    Y[i] = f.eval(&x, ctrl, ctrlClasses, 12);
  }

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot_xy(X, Y);
  plot.show();

  return 0;
}


