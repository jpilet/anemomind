/*
 *  Created on: Apr 23, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Multiplayer.h"
#include <server/math/nonlinear/StepMinimizer.h>
#include <server/math/pareto.h>
#include <assert.h>

namespace sail {


namespace {
  Array<StepMinimizerState> makeInitialStates(Array<std::function<double(double)> >objfs,
    Arrayd X, Arrayd steps) {
    int n = objfs.size();
    assert(n == X.size());
    assert(n == steps.size());
    Array<StepMinimizerState> states(n);
    for (int i = 0; i < n; i++) {
      double x = X[i];
      states[i] = StepMinimizerState(x, steps[i], objfs(x));
    }
    return states;
  }

  ParetoElement evaluateAll(Array<std::function<double(double)> > objfs,
      Arrayd X) {
    int n = objfs.size();
    Arrayd vals(n);
    for (int i = 0; i < n; i++) {
      vals[i] = objfs[i](X[i]);
    }
    return vals;
  }
}


void optimizeMultiplayerSub(StepMinimizer &minimizer,
    Array<std::function<double(double)> > objfs,
      Array<StepMinimizerState> states) {
  ParetoFrontier frontier;
  for (int i = 0; i < minimizer.maxiter(); i++) {
    for (int j = 0; j < n; j++) {
      Arrayd X = states.map<double>([&](StepMinimizerState x) {return x.getX();});
      std::function<bool(double, double)> acc = [&] (double x, double y) {
        X[j] = x;
        return frontier.insert(evaluateAll(objfs, X));
      };
      minimizer.setAcceptor(acc);
      states[j] = minimizer.takeStep(states[j], objfs[j]);
    }
  }
}

Arrayd optimizeMultiplayer(StepMinimizer &minimizer,
    Array<std::function<double(double)> > objfs, Arrayd X,
    Arrayd initStepSizes) {
  int n = objfs.size();
  assert(n == X.size());

  if (initStepSizes.empty()) {
    initStepSizes = Arrayd::fill(n, minimizer.recommendedInitialStep());
  }
  Array<StepMinimizerState> states = makeInitialStates(objfs, X, initStepSizes);
  optimizeMultiplayerSub(minimizer, objfs, states);
  return states.map<double>([&](StepMinimizerState x) {return x.getX();});
}

} /* namespace sail */
