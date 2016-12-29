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

class StepManager {
public:
  virtual void report(
        const Array<double> &X,
        double y,
        const Array<double> &grad) = 0;
  virtual double currentStep() const = 0;
  virtual ~StepManager() {}

  typedef std::shared_ptr<StepManager> Ptr;
};

class ConstantStepManager : public StepManager {
public:
  ConstantStepManager(double s) : _stepSize(s) {}

  void report(
        const Array<double> &X,
        double y,
        const Array<double> &grad) override {}

  double currentStep() const override {
    return _stepSize;
  }
private:
  double _stepSize;
};


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

double evaluatePartialGradient(
    int i, Function f,
    const Array<Array<double>> &X,
    const Settings &settings,
    double *Y) {
  auto Xdep = X[i];
  int dim = Xdep.size();
  trace_on(settings.tapeIndex);
    auto adX = makePartialADX(i, X);
    auto ady = f(adX);
    double y = 0;
    ady >>= y;
  trace_off();
  gradient(settings.tapeIndex, dim, Xdep.getData(), Y);
  return y;
}


Array<double> takePartialStep(
    int i, Function f,
    Array<Array<double>> X,
    const Settings &settings,
    StepManager *manager) {
  auto Xdep = X[i];
  int dim = Xdep.size();
  Array<double> Y(dim);
  double y = evaluatePartialGradient(
      i, f, X, settings, Y.getData());
  manager->report(Xdep, y, Y);
  for (int i = 0; i < dim; i++) {
    Y[i] = Xdep[i] - manager->currentStep()*Y[i];
  }
  return Y;
}

Array<Array<double>> takeStep(
    const Array<Function> &objectives,
    const Array<Array<double>> &X,
    const Array<StepManager::Ptr> &stepManagers,
    const Settings &settings) {
  int n = X.size();
  Array<Array<double>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = takePartialStep(
        i, objectives[i],
        X, settings, stepManagers[i].get());
  }
  return dst;
}

Array<StepManager::Ptr> initializeStepManagers(int n,
    const Settings &settings) {
  Array<StepManager::Ptr> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = StepManager::Ptr(new ConstantStepManager(
        settings.stepSize));
  }
  return dst;
}

Array<Array<double>> optimize(
    Array<Function> objectives,
    Array<Array<double>> initialEstimate,
    const Settings &settings) {
  auto managers = initializeStepManagers(
      objectives.size(), settings);
  auto X = initialEstimate;
  callCallback(settings.iterationCallback, 0, X);
  for (int i = 0; i < settings.iterationCount; i++) {
    X = takeStep(objectives, X, managers, settings);
    callCallback(settings.iterationCallback, i, X);
  }
  return X;
}

}
}
