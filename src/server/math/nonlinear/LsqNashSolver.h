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

int getMaxInputIndex(const Array<int> &inds);

class SubFunction {
public:
  virtual double eval(const double *X) const = 0;
  virtual double eval(const double *X,
        double *JtF,
        std::vector<Eigen::Triplet<double>> *JtJ) const = 0;

  virtual int maxInputIndex() const = 0;
  virtual int JtJElementCount() const = 0;

  virtual ~SubFunction() {}

  typedef std::shared_ptr<SubFunction> Ptr;
};

template <typename F>
class ADSubFunction : public SubFunction {
public:
  typedef ceres::Jet<double, F::inputCount> ADType;

  ADSubFunction(Span<int> strategyRange,
      const Array<int> inputIndices,
      const F &f = F()) :
    _f(f), _inputIndices(inputIndices) {
    CHECK(inputIndices.size() == F::inputCount);
    for (int i = 0; i < F::inputCount; i++) {
      if (strategyRange.contains(inputIndices[i])) {
        _strategyIndexSubset.push_back(i);
      }
    }
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

  int maxInputIndex() const override {
    return getMaxInputIndex(_inputIndices);
  }

  int JtJElementCount() const override {
    return F::inputCount*_strategyIndexSubset.size();
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
    for (int i: _strategyIndexSubset) {
      int I = _inputIndices[i];
      for (int j = 0; j < F::inputCount; j++) {
        int J = _inputIndices[j];
        double sum = 0.0;
        for (int k = 0; k < F::outputCount; k++) {
          sum += Ysrc[k].v[i]*Ysrc[k].v[j];
        }
        JtJ->push_back(Eigen::Triplet<double>(
            I, J, sum));
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
  std::vector<int> _strategyIndexSubset;
  F _f;
};


class Player {
public:
  template <typename F>
  void addADSubFunction(const F &f, const Arrayi &inds) {
    add(
        std::static_pointer_cast<SubFunction>(
            std::make_shared<ADSubFunction<F>>(_strategySpan,
                inds, f)));
  }


  Player() {}


  Player(Spani span);
  void add(SubFunction::Ptr p);
  double eval(const double *X) const;
  double eval(const double *X,
      double *JtFout,
      std::vector<Eigen::Triplet<double>> *JtJout);
  int minInputSize() const;
  int JtJElementCount() const;
  typedef std::shared_ptr<Player> Ptr;
  Spani strategySpan() const;
private:
  int _jacobianElementCount = 0;
  int _minInputSize = 0;

  Spani _strategySpan;
  std::vector<SubFunction::Ptr> _functions;
};

struct Settings {
  int verbosity = 0;
  int iters = 30;
  int subIters = 300;
  double tau = 1.0e-3;
  double e1 = 1.0e-15;
  double e2 = 1.0e-15;
  double e3 = 1.0e-15;
};

struct Results {
  enum TerminationType {
    None = 0,

    Converged = 1,

    // Used all iterations
    MaxIterationsReached = 2,

    // Failures. Probably a bad solution
    FullEvaluationFailed = 3,
    ResidualEvaluationFailed = 4,
    SolveFailed = 5,
    IterationsExceeded = 6,
    InnerStepFailed = 7,
    MuNotFinite = 8
  };

  TerminationType type = None;
  int iterationsCompleted = 0;

  Eigen::VectorXd X;
};

struct State {
  Array<double> values;
  Eigen::VectorXd X;
};

class AcceptancePolicy {
public:
  virtual bool acceptable(
      const Array<Player::Ptr> &players,
      const State &current,
      const State &candidate) const = 0;
  virtual void accept(
      const Array<Player::Ptr> &players,
      const State &candidate) = 0;
  virtual ~AcceptancePolicy() {}
};

class ApproximatePolicy : public AcceptancePolicy {
public:
  bool acceptable(
      const Array<Player::Ptr> &players,
      const State &current,
      const State &candidate) const override;

  void accept(
      const Array<Player::Ptr> &players,
      const State &candidate) override {}
};

bool validInput(
    const Array<Player::Ptr> &players,
    const Eigen::VectorXd &Xinit);

State evaluateState(
    const Array<Player::Ptr> &players,
    const Eigen::VectorXd &X);

Results solve(
    const Array<Player::Ptr> &players,
    const Eigen::VectorXd &Xinit,
    const Settings &settings = Settings());

}
}

#endif /* SERVER_MATH_NONLINEAR_LSQNASHSOLVER_H_ */
