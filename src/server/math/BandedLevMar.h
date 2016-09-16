/*
 * BandedLevMar.h
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 *
 * A templated implementation
 * of the Levenberg-Marquardt problem for banded problems.
 */

#ifndef SERVER_MATH_BANDEDLEVMAR_H_
#define SERVER_MATH_BANDEDLEVMAR_H_

#include <memory>
#include <server/common/Span.h>
#include <vector>
#include <server/common/logging.h>
#include <ceres/jet.h>
#include <Eigen/Dense>
#include <server/math/lapack/BandWrappersJet.h>

namespace sail {
namespace BandedLevMar {

template <int dim, typename T>
T dotProduct(T *a, T *b) {
  T sum = T(0.0);
  for (int i = 0; i < dim; i++) {
    sum += a[i]*b[i];
  }
  return sum;
}

template <typename T>
class CostFunctionBase {
public:
  virtual int inputCount() const = 0;
  virtual int outputCount() const = 0;
  virtual Spani inputRange() const = 0;
  virtual bool evaluate(const T *X, T *outLocal) = 0;
  virtual bool evaluate(const T *X, T *Y, T *JcolMajor) = 0;
  virtual bool accumulateNormalEquations(
      const T *X,
      SymmetricBandMatrixL<T> *JtJ,
      MDArray<T, 2> *minusJtF) = 0;
};

template <typename CostEvaluator, typename T>
class UniqueCostFunction : public CostFunctionBase<T> {
public:
  typedef ceres::Jet<T, 1> ADType;

  UniqueCostFunction(
      const Spani &inputRange,
      CostEvaluator *f) :
        _inputRange(inputRange),
        _f(f) {
    CHECK(_inputRange.width() == CostEvaluator::inputCount);
  }

  Spani inputRange() const override {
    return _inputRange;
  }

  int outputCount() const override {
    return CostEvaluator::outputCount;
  }

  int inputCount() const override {
    return CostEvaluator::inputCount;
  }

  bool evaluate(const T *X, T *outLocal) override {
    return _f->evaluate(X + _inputRange.minv(), outLocal);
  }

  bool evaluate(const T *X, T *Y, T *J) override {
    ADType _adX[CostEvaluator::inputCount];
    ADType _adY[CostEvaluator::outputCount];
    for (int i = 0; i < CostEvaluator::inputCount; i++) {
      _adX[i] = ADType(X[i]);
    }
    for (int i = 0; i < CostEvaluator::inputCount; i++) {
      _adX[i].v[0] = 1.0;
      if (!_f->template evaluate<ceres::Jet<T, 1> >(_adX, _adY)) {
        return false;
      }
      _adX[i].v[0] = 0.0;

      for (int j = 0; j < CostEvaluator::outputCount; j++) {
        Y[j] = _adY[j].a;
        J[j] = _adY[j].v[0];
      }
      J += CostEvaluator::outputCount;
    }
    return true;
  }

  bool accumulateNormalEquations(
      const T *X, SymmetricBandMatrixL<T> *JtJ,
      MDArray<T, 2> *minusJtF) override {
    T F[CostEvaluator::inputCount];
    T J[CostEvaluator::inputCount*CostEvaluator::outputCount];
    if (!evaluate(X, F, J)) {
      return false;
    }

    int offset = _inputRange.minv();

    // TODO: Wrap it with Eigen::Map, so that
    // we can use Eigen instead of our home-made matrix multiplication...
    for (int i = 0; i < CostEvaluator::inputCount; i++) {
      T *j0 = J + i*CostEvaluator::outputCount;
      (*minusJtF)(offset + i, 0) -=
          dotProduct<CostEvaluator::outputCount>(j0, F);
      for (int j = 0; j < CostEvaluator::inputCount; j++) {
        T *j1 = J + j*CostEvaluator::outputCount;
        JtJ->add(offset + i, offset + j,
            dotProduct<CostEvaluator::outputCount>(j0, j1));
        std::cout << "Adding to " << offset+i << ", "
            << offset+j << std::endl;
        std::cout << "JtJ is now \n " << JtJ->storage() << "\n\n";
      }
    }
    return true;
  }
private:
  Spani _inputRange;
  std::unique_ptr<CostEvaluator> _f;
};

template <typename T>
class Problem {
public:
  Problem(int expectedCostFunctionCount = -1) : _kd(0),
    _paramCount(0), _residualCount(0) {
    if (expectedCostFunctionCount > 0) {
      _costFunctions.reserve(expectedCostFunctionCount);
    }
  }

  template <typename CostEvaluator>
  void addCostFunction(Spani inputRange,
      CostEvaluator *f) {
    std::unique_ptr<CostFunctionBase<T>> cost(
        new UniqueCostFunction<CostEvaluator, T>(inputRange, f));
    addCost(cost);
  }

  int kd() const {return _kd;}
  int paramCount() const {return _paramCount;}
  int residualCount() const {return _residualCount;}

  bool fillNormalEquations(
      const T *X,
      SymmetricBandMatrixL<T> *JtJ, MDArray<T, 2> *minusJtF) const {
    *JtJ = SymmetricBandMatrixL<T>::zero(_paramCount, _kd);
    *minusJtF = MDArray<T, 2>(_paramCount, 1);
    minusJtF->setAll(T(0.0));
    for (auto &f: _costFunctions) {
      if (!f->accumulateNormalEquations(X, JtJ, minusJtF)) {
        return false;
      }
    }
    return true;
  }

  bool evaluate(const T *X, T *dst) const {
    int offset = 0;
    for (const auto &f: _costFunctions) {
      if (!f->evaluate(X, dst + offset)) {
        return false;
      }
      offset += f->outputCount();
    }
    assert(offset == _residualCount);
    return true;
  }
private:
  int _kd, _paramCount, _residualCount;
  std::vector<std::unique_ptr<CostFunctionBase<T> > > _costFunctions;

  void addCost(std::unique_ptr<CostFunctionBase<T>> &cost) {
    _kd = std::max(_kd, cost->inputCount()-1);
    _paramCount = std::max(_paramCount, cost->inputRange().maxv());
    _residualCount += cost->outputCount();
    _costFunctions.push_back(std::move(cost));
  }
};

struct Settings {
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

    //
    Converged,

    // Used all iterations
    MaxIterationsReached,

    // Failures. Probably a bad solution
    FullEvaluationFailed,
    ResidualEvaluationFailed,
    PbsvFailed,
  };

  TerminationType type = None;
  int lastCompletedIteration = -1;
};

template <typename T>
T getMaxDiagElement(const SymmetricBandMatrixL<T> &A) {
  int n = A.size();
  T maxElement = T(A.diagonalElement(0));
  for (int i = 1; i < n; i++) {
    maxElement = std::max(maxElement, A.diagonalElement(i));
  }
  return maxElement;
}

template <typename T>
void addDamping(T amount, SymmetricBandMatrixL<T> *dst) {
  int n = dst->size();
  for (int i = 0; i < n; i++) {
    dst->add(i, i, amount);
  }
}

template <typename T>
Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, 1> >
  wrapEigen(const Array<T> &x) {
  return Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, 1> >(x.ptr(), x.size());
}

template <typename T>
Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, 1> >
  wrapEigen1(const MDArray<T, 2> &x) {
  assert(x.cols() == 1);
  assert(x.isContinuous());
  return Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, 1> >(
      x.ptr(), x.rows());
}

template <typename T>
using Vec = Eigen::Matrix<T, Eigen::Dynamic, 1>;

template <typename T>
T acceptedUpdateFactor(T rho) {
  T k = T(2.0)*rho - T(1.0);
  return std::max(T(1.0/3), T(1.0) - k*k*k);
}


template <typename T>
T maxAbs(const MDArray<T, 2> &X) {
  assert(X.cols() == 1);
  int n = X.rows();
  T maxv(0.0);
  for (int i = 0; i < n; i++) {
    maxv = std::max(maxv, fabs(X(i, 0)));
  }
  return maxv;
}

// Implemented closely according to
// http://users.ics.forth.gr/~lourakis/levmar/levmar.pdf
template <typename T>
Results runLevmar(
    const Settings &settings,
    const Problem<T> &problem,
    Eigen::Matrix<T, Eigen::Dynamic, 1> *X) {
  Results results;
  T v = T(2.0);
  T mu = T(-1.0);

  Vec<T> residuals(problem.residualCount());

  if (!problem.evaluate(X->data(), residuals.data())) {
    results.type = Results::ResidualEvaluationFailed;
    return results;
  }

  SymmetricBandMatrixL<T> JtJ0;
  MDArray<T, 2> minusJtF0;
  for (int i = 0; i < settings.iters; i++) {
    if (problem.fillNormalEquations(X->data(), &JtJ0, &minusJtF0)) {
      results.type = Results::FullEvaluationFailed;
      return results;
    }

    if (i == 0) {
      mu = settings.tau*getMaxDiagElement(JtJ0);
    }

    bool found = false;
    for (int i = 0; i < settings.subIters; i++) {
      auto JtJ = JtJ0.dup();
      auto minusJtF = minusJtF0.dup();

      addDamping(mu, &JtJ);
      if (!Pbsv<T>::apply(&JtJ, &minusJtF)) {
        results.type = Results::PbsvFailed;
        return results;
      }

      T xNorm = X->norm();
      auto eigenStep = wrapEigen1(minusJtF);
      if (eigenStep.norm() < settings.e2*xNorm) {
        results.type = Results::Converged;
        return results;
      }

      T oldResidualNorm = residuals.norm();
      Vec<T> xNew = *X + eigenStep;
      Vec<T> newResiduals(problem.residualCount());
      if (!problem.evaluate(xNew.data(), newResiduals.data())) {
        results.type = Results::ResidualEvaluationFailed;
        return results;
      }

      // Is positive when improved:
      T improvement = oldResidualNorm - newResiduals.norm();
      auto eigenMinusJtF = wrapEigen1(minusJtF);
      T denom = eigenStep.dot(mu*eigenStep + eigenMinusJtF);
      T rho = improvement/denom;
      if (T(0.0) < rho) {
        residuals = newResiduals;
        *X = xNew;
        if (maxAbs(minusJtF) < settings.e1
            || residuals.norm() < settings.e3) {
          results.type = Results::Converged;
          return results;
        }
        mu *= acceptedUpdateFactor(rho);
        v = T(2.0);

        bool found = true;
        break;
      } else {
        mu *= v;
        v *= T(2.0);
      }
    }
    results.lastCompletedIteration = i;
  }
  results.type = Results::MaxIterationsReached;
  return results;
}

}
}




#endif /* SERVER_MATH_BANDEDLEVMAR_H_ */
