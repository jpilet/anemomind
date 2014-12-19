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

  int ruleCount = 5;
  MDArray<Rule, 2> rules(ruleCount, ruleCount);

  double marg = 0.2;
  std::uniform_real_distribution<double> balanceDistrib(0, 1);
  double s = 0.1;
  std::uniform_real_distribution<double> scalingDistrib(-s, s);
  std::uniform_real_distribution<double> irregDistrib(-1, 1);
  std::uniform_int_distribution<int> indexDistrib(0, ruleCount-1);
  for (int i = 0; i < ruleCount; i++) {
    for (int j = 0; j < ruleCount; j++) {
      rules(i, j) = Rule(balanceDistrib(e), scalingDistrib(e),
          indexDistrib(e), irregDistrib(e));
    }
  }

  typedef Vertex<double> Vertexd;

  Vertexd ctrl[4] = {Vertexd(0, 1, 0), Vertexd(0, 1, 1), Vertexd(0, 1, 2), Vertexd(0, 1, 3)};

  SubdivFractals<1> f(rules);

  int sampleCount = 3000;
  Arrayd X(sampleCount);
  Arrayd Y(sampleCount);
  for (int i = 0; i < sampleCount; i++) {
    double x = double(i)/sampleCount;
    X[i] = x;
    double x2[2] = {x, x};
    Y[i] = f.eval(x2, ctrl, 12);
  }

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot_xy(X, Y);
  plot.show();

  return 0;
}


