/*
 * GameSolver.cpp
 *
 *  Created on: 29 Dec 2016
 *      Author: jonas
 */

#include <server/math/nonlinear/GameSolver.h>
#include <server/common/logging.h>
#include <adolc/taping.h>
#include <adolc/drivers/drivers.h>

namespace sail {
namespace GameSolver {

namespace {
  void callCallback(IterationCallback cb,
      int iter, const Array<Array<double>> &X) {
    if (cb) {
      cb(iter, X);
    }
  }

  Array<adouble> wrapDependent(const Array<double> &src) {
    int n = src.size();
    Array<adouble> dst(n);
    for (int i = 0; i < n; i++) {
      dst[i] <<= src[i];
    }
    return dst;
  }

  Array<adouble> wrapIndependent(const Array<double> &src) {
    int n = src.size();
    Array<adouble> dst(n);
    for (int i = 0; i < n; i++) {
      dst[i] = src[i];
    }
    return dst;
  }

  Array<Array<adouble>> makePartialADX(
      int dependent,
      const Array<Array<double>> &src) {
    int n = src.size();
    Array<Array<adouble>> dst(n);
    for (int i = 0; i < n; i++) {
      dst[i] = i == dependent?
          wrapDependent(src[i])
          : wrapIndependent(src[i]);
    }
    return dst;
  }

  Array<double> takePartialStep(
      int i, Function f,
      Array<Array<double>> X,
      const Settings &settings) {

    trace_on(settings.tapeIndex);
      auto adX = makePartialADX(i, X);
      auto ady = f(adX);
      double y = 0;
      ady >>= y;
    trace_off();

    auto Xdep = X[i];
    int dim = Xdep.size();
    Array<double> Y(dim);
    gradient(settings.tapeIndex, dim, Xdep.getData(), Y.getData());
    for (int i = 0; i < dim; i++) {
      Y[i] = Xdep[i] - settings.stepSize*Y[i];
    }
    return Y;
  }

  Array<Array<double>> takeStep(
      const Array<Function> &objectives,
      const Array<Array<double>> &X,
      const Settings &settings) {
    int n = X.size();
    Array<Array<double>> dst(n);
    for (int i = 0; i < n; i++) {
      dst[i] = takePartialStep(i, objectives[i], X, settings);
    }
    return dst;
  }
}

Array<Array<double>> optimize(
    Array<Function> objectives,
    Array<Array<double>> initialEstimate,
    const Settings &settings) {
  auto X = initialEstimate;
  callCallback(settings.iterationCallback, 0, X);
  for (int i = 0; i < settings.iterationCount; i++) {
    X = takeStep(objectives, X, settings);
    callCallback(settings.iterationCallback, i, X);
  }
  return X;
}

}
}
