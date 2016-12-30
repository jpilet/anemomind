/*
 * GameSolver.h
 *
 *  Created on: 29 Dec 2016
 *      Author: jonas
 *
 * Implementation of Eq. 36 in
 *
@inproceedings{ratliff_characterization_2013,
  title = {Characterization and computation of local nash equilibria in continuous games},
  url = {http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=6736623},
  urldate = {2016-12-29},
  booktitle = {Communication, {Control}, and {Computing} ({Allerton}), 2013 51st {Annual} {Allerton} {Conference} on},
  publisher = {IEEE},
  author = {Ratliff, Lillian J. and Burden, Samuel A. and Sastry, S. Shankar},
  year = {2013},
  pages = {917--924},
  file = {RatliffCharacterization2013.pdf:/Users/jonas/Library/Application Support/Firefox/Profiles/71ax27z9.default/zotero/storage/EHBGD8G4/RatliffCharacterization2013.pdf:application/pdf}
}

Use Armijo Rule?
https://en.wikipedia.org/wiki/Wolfe_conditions

This seems easier:
http://www.onmyphd.com/?p=gradient.descent#h4_barzilaiborwein

 */

#ifndef SERVER_MATH_NONLINEAR_GAMESOLVER_H_
#define SERVER_MATH_NONLINEAR_GAMESOLVER_H_

#include <server/common/Array.h>
#include <adolc/adouble.h>
#include <server/common/RNG.h>
#include <server/common/Optional.h>

namespace sail {
namespace GameSolver {

class StepManager {
public:
  typedef std::shared_ptr<StepManager> Ptr;

  virtual void report(
        const Array<double> &X,
        double y,
        const Array<double> &grad) = 0;
  virtual StepManager::Ptr dup() = 0;
  virtual double currentStep() = 0;
  virtual ~StepManager() {}
  virtual void usedStep(double step) = 0;
};

class ConstantStepManager : public StepManager {
public:
  ConstantStepManager(double s) : _stepSize(s) {}

  void report(
        const Array<double> &X,
        double y,
        const Array<double> &grad) override {}

  double currentStep() override {
    return _stepSize;
  }

  StepManager::Ptr dup() override {
    return StepManager::Ptr(new ConstantStepManager(_stepSize));
  }

  void usedStep(double) override {}
private:
  double _stepSize;
};


class RandomStepManager : public StepManager {
public:
  struct Settings {
    bool verbose = false;
    int initialSampleCount = 12;
    int subSampleSize = 3;
    double filterSharpness = 0.1;
    int filterIterations = 1;
    double logInitialStepMu = 0.0;
    double logInitialStepSigma = 1.0;
    sail::RNG *rng = nullptr;
  };

  struct StepAndValue {
    StepAndValue() {}
    StepAndValue(double x, double y) : logStep(x), logValue(y) {}
    double logStep = 0.0;
    double logValue = 0.0;
    bool operator<(const StepAndValue &other) const {
      return logValue < other.logValue;
    }
  };

  RandomStepManager(const Settings &s);
  void report(
        const Array<double> &X,
        double y,
        const Array<double> &grad) override;
  double currentStep() override;
  void usedStep(double stepSize) override;
  StepManager::Ptr dup() override;
private:
  Settings _settings;
  Optional<double> _logLastStep;
  std::vector<StepAndValue> _values;
  std::vector<double> _rawObjfValues;
};


typedef std::function<adouble(Array<Array<adouble>>)> Function;
typedef std::function<void(int, Array<Array<double>>)> IterationCallback;

struct Settings {
  bool verbose = false;
  int iterationCount = 120;
  StepManager::Ptr stepManagerPrototype;
  IterationCallback iterationCallback;
  short tapeIndex = 0;
};

double evaluatePartialGradient(
    int i, Function f,
    const Array<Array<double>> &X,
    const Settings &settings,
    double *Y);

Array<Array<double>> optimize(
    Array<Function> objectives,
    Array<Array<double>> initialEstimate,
    const Settings &settings);


}
}

#endif /* SERVER_MATH_NONLINEAR_GAMESOLVER_H_ */
