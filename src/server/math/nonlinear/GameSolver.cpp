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
#include <random>
#include <server/common/MeanAndVar.h>

namespace sail {
namespace GameSolver {

RandomStepManager::RandomStepManager(
    const RandomStepManager::Settings &s)
  : _settings(s) {
  CHECK(s.rng);
  std::normal_distribution<double> Xd(
      s.logInitialStepMu, s.logInitialStepSigma);
  std::normal_distribution<double> Yd(0.0, 1.0e-4);
  for (int i = 0; i < s.initialSampleCount; i++) {
    _values.push_back(RandomStepManager::StepAndValue{
      Xd(*(s.rng)), Yd(*(s.rng))
    });
  }
}

void RandomStepManager::report(
      const Array<double> &X,
      double y,
      const Array<double> &grad) {
  _rawObjfValues.push_back(y);
  if (2 <= _rawObjfValues.size() && _logLastStep.defined()) {
    int n = _rawObjfValues.size();
    double change =
        log(_rawObjfValues[n-1]) - log(_rawObjfValues[n-2]);
    _values.push_back({_logLastStep.get(), change});
  }
}

Array<double> extractValues(
    const std::vector<RandomStepManager::StepAndValue> &src) {
  int n = src.size();
  Array<double> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = src[i].logValue;
  }
  return dst;
}

Array<RandomStepManager::StepAndValue> applyFilteredValues(
    const std::vector<RandomStepManager::StepAndValue> &src,
    const Array<double> &newValues) {
  CHECK(src.size() == newValues.size());
  int n = src.size();
  Array<RandomStepManager::StepAndValue> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = {src[i].logStep, newValues[i]};
  }
  return dst;
}

Array<double> applyLocalFilter(const Array<double> &src,
    std::function<double(double, double, double)> f) {
  int n = src.size();
  if (n <= 1) {
    return src;
  } else {
    Array<double> dst(n);
    dst[0] = f(src[1], src[0], src[1]);
    dst[n-1] = f(src[n-2], src[n-1], src[n-2]);
    for (int i = 1; i < n-1; i++) {
      dst[i] = f(src[i-1], src[i], src[i+1]);
    }
    return dst;
  }
}

Array<double> filter(const Array<double> &src,
    const RandomStepManager::Settings &settings) {
  return applyLocalFilter(src, [&](double a, double b, double c) {
    return std::max(b,
        std::max(a - settings.filterSharpness,
                 b - settings.filterSharpness));
  });
}

Array<RandomStepManager::StepAndValue> filter(
    const std::vector<RandomStepManager::StepAndValue> &src,
    const RandomStepManager::Settings &settings) {
  auto values = extractValues(src);
  for (int i = 0; i < settings.filterIterations; i++) {
    values = filter(values, settings);
  }
  return applyFilteredValues(src, values);
}

double RandomStepManager::currentStep() {
  CHECK(_settings.subSampleSize <= _values.size());
  auto filtered = filter(_values, _settings);
  std::sort(filtered.begin(), filtered.end());
  MeanAndVar acc;
  for (auto x: filtered.sliceTo(_settings.subSampleSize)) {
    acc.add(x.logStep);
  }
  std::normal_distribution<double> distrib(
      acc.mean(), acc.standardDeviation());
  _logLastStep = distrib(*(_settings.rng));

  double y = exp(_logLastStep.get());
  std::cout << "  propose step " << y << std::endl;
  return y;
}

void RandomStepManager::usedStep(double stepSize) {
  _logLastStep = log(stepSize);
}

StepManager::Ptr RandomStepManager::dup() {
  return StepManager::Ptr(new RandomStepManager(_settings));
}


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
    double *Y = nullptr) {
  auto Xdep = X[i];
  int dim = Xdep.size();

  if (Y != nullptr) {trace_on(settings.tapeIndex);}
    auto adX = makePartialADX(i, X);
    auto ady = f(adX);
    double y = 0;
    ady >>= y;
  if (Y != nullptr) {
    trace_off();
    gradient(settings.tapeIndex, dim, Xdep.getData(), Y);
  }
  return y;
}

Array<double> applyStep(const Array<double> &X, double h,
    const Array<double> &grad) {
  int n = X.size();
  Array<double> Y(n);
  for (int i = 0; i < n; i++) {
    Y[i] = X[i] - h*grad[i];
  }
  return Y;
}




Array<double> takePartialStep(
    int i, Function f,
        Array<Array<double>> X,
        const Settings &settings,
        StepManager *manager) {
  auto Xdep = X[i];
  int dim = Xdep.size();
  Array<double> grad(dim);
  double current = evaluatePartialGradient(
      i, f, X, settings, grad.getData());
  manager->report(Xdep, current, grad);
  auto Xtrial = X.dup();
  auto h = manager->currentStep();
  while (true) {
    Xtrial[i] = applyStep(Xdep, h, grad);
    auto next = evaluatePartialGradient(i, f, Xtrial, settings);
    std::cout << "  current=" << current << " next=" << next << " h=" << h << std::endl;
    if (next <= current) {
      break;
    }
    h *= 0.5;
  }
  manager->usedStep(h);
  return Xtrial[i];
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
    dst[i] = settings.stepManagerPrototype->dup();
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
