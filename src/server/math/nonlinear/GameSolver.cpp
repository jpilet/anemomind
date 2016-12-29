/*
 * GameSolver.cpp
 *
 *  Created on: 29 Dec 2016
 *      Author: jonas
 */

#include <server/math/nonlinear/GameSolver.h>

namespace sail {
namespace GameSolver {

namespace {
  void callCallback(IterationCallback cb,
      int iter, const Array<Array<double>> &X) {
    if (cb) {
      cb(iter, X);
    }
  }

  Array<Array<double>> takeStep(
      const Array<Function> &objectives,
      const Array<Array<double>> &X,
      double h) {

  }
}

Array<Array<double>> optimize(
    Array<Function> objectives,
    Array<Array<double>> initialEstimate,
    const Settings &settings) {
  auto X = initialEstimate;
  callCallback(settings.iterationCallback, 0, X);
  for (int i = 0; i < settings.iterationCount; i++) {
    X = takeStep(objectives, X, settings.stepSize);
    callCallback(settings.iterationCallback, i, X);
  }
  return X;
}

}
}
