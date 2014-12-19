/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <random>
#include <server/plot/extra.h>
#include <server/math/SubdivFractals.h>

using namespace sail;

int main() {
  constexpr int Dim = 2;

  typedef SubdivFractals<Dim> Frac;

  std::default_random_engine e;

  int ruleCount = 5;
  MDArray<Rule, 2> rules(ruleCount, ruleCount);

  MaxSlope slope(300.0, 300.0);
  std::uniform_real_distribution<double> alphaBetaDistrib(-1, 1);
  std::uniform_int_distribution<int> indexDistrib(0, ruleCount-1);
  std::cout << "RULES:" << std::endl;
  for (int i = 0; i < ruleCount; i++) {
    for (int j = 0; j < ruleCount; j++) {
      auto rule = Rule(slope,
          alphaBetaDistrib(e),
          alphaBetaDistrib(e),
          indexDistrib(e));
      rules(i, j) = rule;
      std::cout << rule << " ";
    }
    std::cout << std::endl;
  }



  Vertex ctrl[Frac::ctrlCount()];
  for (int i = 0; i < Frac::ctrlCount(); i++) {
    ctrl[i] = Vertex(0, i % ruleCount);
  }


  SubdivFractals<1> f(rules);

  int sampleCount = 4000;
  Arrayd X(sampleCount);
  Arrayd Y(sampleCount);
  //for (int i = 0; i < sampleCount; i++) {
  {
    int i = 1000;
    double x = double(i)/sampleCount;
    X[i] = x;
    double xcoord[Dim];
    for (int j = 0; j < Dim; j++) {
      xcoord[j] = x;
    }
    Y[i] = f.eval(xcoord, ctrl, 2);
  }

//  GnuplotExtra plot;
//  plot.set_style("lines");
//  plot.plot_xy(X, Y);
//  plot.show();

  return 0;
}


