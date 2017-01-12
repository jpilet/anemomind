/*
 * LsqNashSolver.h
 *
 *  Created on: 6 Jan 2017
 *      Author: jonas
 */

#ifndef SERVER_MATH_NONLINEAR_LSQNASHSOLVER_H_
#define SERVER_MATH_NONLINEAR_LSQNASHSOLVER_H_

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <ceres/jet.h>
#include <vector>
#include <server/common/Span.h>
#include <server/common/math.h>
#include <server/common/logging.h>

namespace sail {
namespace LsqNashSolver {

class SubFunction {
public:
  virtual double eval(const double *X) const = 0;
  virtual double eval(const double *X,
        double *JtF,
        std::vector<Eigen::Triplet<double>> *JtJ) const = 0;

  virtual int inputCount() const = 0;
  virtual int outputCount() const = 0;

  virtual bool hasValidStrategy(Spani strategySpan) const = 0;

  virtual ~SubFunction() {}
};

template <typename F,
  int StrategyLowerBound,
  int StrategyUpperBound>
class ADSubFunction : public SubFunction {
public:
  static_assert(StrategyLowerBound < StrategyUpperBound,
      "An empty strategy is not so interesting");
  static_assert(0 <= StrategyLowerBound,
      "Lower bound too low");
  static_assert(StrategyUpperBound <= F::inputCount,
      "Upper bound cannot exceed the number of inputs to F");

  typedef ceres::Jet<double, F::inputCount> ADType;

  ADSubFunction(const Array<int> inputIndices,
      const F &f = F()) :
    _f(f), _inputIndices(inputIndices) {
    CHECK(inputIndices.size() == F::inputCount);
  }

  double eval(const double *X) const override {
    double Xlocal[F::inputCount];
    double Ylocal[F::outputCount];
    input(X, Xlocal);
    _f.template eval<double>(Xlocal, Ylocal);
    return computeSquaredResidualNorm(Ylocal);
  }

  double eval(const double *X,
      double *JtF,
      std::vector<Eigen::Triplet<double>> *JtJ) const override {
    ADType Xlocal[F::inputCount];
    ADType Y[F::outputCount];
    input(X, Xlocal);
    _f.template eval<ADType>(Xlocal, Y);
    return output(Y, JtF, JtJ);
  }

  int inputCount() const override {
    return F::inputCount;
  }

  int outputCount() const override {
    return F::outputCount;
  }

  bool hasValidStrategy(Spani strategySpan) const override {
      for (auto index: _inputIndices) {
      if (!strategySpan.contains(index)) {
        return false;
      }
    }
    return true;
  }
private:
  void input(const double *src, double *dst) const {
    for (int i = 0; i < F::inputCount; i++) {
      dst[i] = src[_inputIndices[i]];
    }
  }

  void input(const double *src, ADType *dst) const {
    for (int i = 0; i < F::inputCount; i++) {
      dst[i] = ADType(src[_inputIndices[i]], i);
    }
  }

  double computeSquaredResidualNorm(const double *X) const {
    double sum = 0.0;
    for (int i = 0; i < F::outputCount; i++) {
      sum += sqr(X[i]);
    }
    return sum;
  }

  double output(const ADType *Ysrc,
      double *JtF,
      std::vector<Eigen::Triplet<double>> *JtJ) const {
    double sum = 0.0;
    for (int i = 0; i < F::outputCount; i++) {
      sum += sqr(Ysrc[i].a);
    }
    for (int i = StrategyLowerBound; i < StrategyUpperBound; i++) {
      int I = _inputIndices[i];
      for (int j = 0; j < F::inputCount; j++) {
        int J = _inputIndices[j];
        for (int k = 0; k < F::outputCount; k++) {
          JtJ->push_back(Eigen::Triplet<double>(
              I, J, Ysrc[k].v[i]*Ysrc[k].v[j]));
        }
      }
      double sum = 0.0;
      for (int j = 0; j < F::outputCount; j++) {
        const auto &x = Ysrc[j];
        sum += x.v[i]*x.a;
      }
      JtF[I] += sum;
    }
    return sum;
  }

  Array<int> _inputIndices;
  F _f;
};

}
}

#endif /* SERVER_MATH_NONLINEAR_LSQNASHSOLVER_H_ */
