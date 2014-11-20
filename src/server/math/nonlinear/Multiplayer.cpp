/*
 *  Created on: 2014-04-23
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Multiplayer.h"
#include <server/math/nonlinear/StepMinimizer.h>
#include <server/math/pareto.h>
#include <assert.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>
#include <server/common/ScopedLog.h>

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
      states[i] = StepMinimizerState(x, steps[i], objfs[i]->evalScalar(X.ptr()));
    }
    return states;
  }

  ParetoElement evaluateAll(Array<std::shared_ptr<Function> > objfs,
      Arrayd X) {
    int n = objfs.size();
    Arrayd vals(n);
    for (int i = 0; i < n; i++) {
      vals[i] = objfs[i]->evalScalar(X.ptr());
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

namespace {
  Arrayd mapStatesToX(Array<StepMinimizerState> states) {
    return states.map<double>([&](StepMinimizerState x) {return x.getX();});
  }
}


void optimizeMultiplayerSub(StepMinimizer &minimizer,
    Array<std::shared_ptr<Function> > objfs,
    Array<StepMinimizerState> *statesIO) {
  ENTER_FUNCTION_SCOPE;
  int n = statesIO->size();
  assert(n == objfs.size());

  assert(areNToScalarFunctions(n, objfs));
  ParetoFrontier frontier;
  for (int i = 0; i < minimizer.maxiter(); i++) {
    ENTERSCOPE(stringFormat("Iteration %d/%d", i+1, minimizer.maxiter()));
    for (int j = 0; j < n; j++) {
      ENTERSCOPE(stringFormat("Optimize player %d/%d", j+1, n));
      Arrayd XlocalTemp = mapStatesToX(*statesIO);
      std::function<bool(double, double)> acc = [&] (double x, double y) {
        XlocalTemp[j] = x;
        return frontier.accepts(evaluateAll(objfs, XlocalTemp));
      };
      minimizer.setAcceptor(acc);
      std::function<double(double)> objf = [&] (double x) {
        XlocalTemp[j] = x;
        return objfs[j]->evalScalar(XlocalTemp.ptr());
      };
      (*statesIO)[j] = minimizer.takeStep((*statesIO)[j].reevaluate(objf), objf);
      frontier.insert(evaluateAll(objfs, mapStatesToX(*statesIO)));
      SCOPEDMESSAGE(INFO, stringFormat("X         = %.3g", (*statesIO)[j].getX()));
      SCOPEDMESSAGE(INFO, stringFormat("value     = %.3g", (*statesIO)[j].getValue()));
      SCOPEDMESSAGE(INFO, stringFormat("Step size = %.3g", (*statesIO)[j].getStep()));
    }
  }
}


Arrayd optimizeMultiplayer(const StepMinimizer &minimizerIn,
    Array<std::shared_ptr<Function> > objfs, Arrayd X,
    Arrayd initStepSizes) {
  StepMinimizer minimizer = minimizerIn;

  if (initStepSizes.empty()) {
    initStepSizes = Arrayd::fill(X.size(), minimizer.recommendedInitialStep());
  }
  Array<StepMinimizerState> states = makeInitialStates(objfs, X, initStepSizes);
  optimizeMultiplayerSub(minimizer, objfs, &states);
  return states.map<double>([&](StepMinimizerState x) {return x.getX();});
}

} /* namespace sail */
