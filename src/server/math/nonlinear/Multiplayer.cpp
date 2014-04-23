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
  Array<StepMinimizerState> makeInitialStates(Array<std::shared_ptr<Function> > objfs,
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

  ParetoElement evaluateAll(Array<std::shared_ptr<Function> > objfs,
      Arrayd X) {
    int n = objfs.size();
    Arrayd vals(n);
    for (int i = 0; i < n; i++) {
      vals[i] = objfs[i]->evaluateScalar(X);
    }
    return vals;
  }
}



namespace {
  bool areNToScalarFunctions(int n, Array<std::shared_ptr<Function> > funs) {
    assert(n == funs.size());
    for (int i = 0; i < n; i++) {
      if (funs[i]->inDims() != n) {
        return false;
      }
      if (funs[i]->outDims() != 1) {
        return false;
      }
    }
    return true;
  }
}

void optimizeMultiplayerSub(StepMinimizer &minimizer,
    Array<std::shared_ptr<Function> > objfs,
    Array<StepMinimizerState> *statesIO) {
  Array<StepMinimizerState> &states = *statesIO;

  assert(areNToScalarFunctions(n, objfs));
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


Arrayd optimizeMultiplayer(const StepMinimizer &minimizerIn,
    Array<std::shared_ptr<Function> > objfs, Arrayd X,
    Arrayd initStepSizes) {
  StepMinimizer minimizer = minimizerIn;

  int n = X.size();
  assert(n == objfs.size());



  if (initStepSizes.empty()) {
    initStepSizes = Arrayd::fill(n, minimizer.recommendedInitialStep());
  }
  Array<StepMinimizerState> states = makeInitialStates(objfs, X, initStepSizes);
  optimizeMultiplayerSub(minimizer, objfs, &states);
  return states.map<double>([&](StepMinimizerState x) {return x.getX();});
}

} /* namespace sail */
