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
#include <server/common/math.h>
#include <random>
#include <server/math/JetUtils.h>
#include <server/common/ArrayIO.h>

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
  typedef std::function<void(int, const T*)> Callback;

  Spani inputRange;
  int inputCount() const {return inputRange.width();}

  CostFunctionBase(Spani ir) : inputRange(ir) {}

  virtual bool accumulateCost(
      const T *Xlocal, T *totalCost) = 0;
  virtual bool accumulateNormalEquations(
      const T *Xlocal,
      SymmetricBandMatrixL<T> *JtJ,
      MDArray<T, 2> *minusJtF,
      T *totalCost) = 0;

  void addIterationCallback(Callback cb) {
    _callbacks.push_back(cb);
  }

  void callIterationCallbacks(int iteration, const T *data) const {
    for (auto cb: _callbacks) {
      cb(iteration, data + inputRange.minv());
    }
  }

  virtual ~CostFunctionBase() {}
private:
  std::vector<Callback> _callbacks;
};


template <typename T, int outputCount>
struct WithCostOutput {
  T F[outputCount];

  void done(T *totalCost) {
    T sum = MakeConstant<T>::apply(0.0);
    for (int i = 0; i < outputCount; i++) {
      sum += sqr(F[i]);
    }
    *totalCost += sum;
  }
};

template <typename T, int inputCount>
struct ADInputVector {
  typedef ceres::Jet<T, inputCount> ADType;
  ADType X[inputCount];

  ADInputVector(const T *srcX) {
    for (int i = 0; i < inputCount; i++) {
      auto x = ADType(srcX[i]);
      x.v[i] = T(1.0);
      X[i] = x;
    }
  }
};


template <typename T, int outputCount, int inputCount>
struct WithJacobianOutput {
  typedef ceres::Jet<T, inputCount> ADType;
  ADType F[outputCount];

  void done(T *Fout, T *Jout) const {
    for (int i = 0; i < outputCount; i++) {
      auto y = F[i];
      Fout[i] = y.a;
      auto Jsub = Jout + i;
      for (int j = 0; j < inputCount; j++) {
        Jsub[j*outputCount] = y.v[j];
      }
    }
  }

  void done(
      int offset,
      SymmetricBandMatrixL<T> *JtJ,
      MDArray<T, 2> *minusJtF, T *totalCost) {
    T Ftmp[outputCount];
    T Jtmp[outputCount*inputCount];

    done(Ftmp, Jtmp);

    for (int i = 0; i < outputCount; i++) {
      T f = Ftmp[i];
      *totalCost += f*f;
    }

    // TODO: Wrap it with Eigen::Map, so that
    // we can use Eigen instead of our home-made matrix multiplication...
    for (int i = 0; i < inputCount; i++) {
      T *j0 = Jtmp + i*outputCount;
      (*minusJtF)(offset + i, 0) -=
          dotProduct<outputCount>(j0, Ftmp);
      for (int j = 0; j < inputCount; j++) {
        T *j1 = Jtmp + j*outputCount;
        JtJ->add(offset + i, offset + j,
            dotProduct<outputCount>(j0, j1));
      }
    }
  }
};

template <typename CostEvaluator, typename T>
class SharedCostFunction : public CostFunctionBase<T> {
public:
  static const int N = CostEvaluator::inputCount;
  typedef ceres::Jet<T, N> ADType;

  SharedCostFunction(
      const Spani &inputRange,
      const std::shared_ptr<CostEvaluator> &f) :
        CostFunctionBase<T>(inputRange),
        _f(f) {
    CHECK(inputRange.width() == CostEvaluator::inputCount);
  }

  bool accumulateCost(const T *Xlocal, T *totalCost) override {
    WithCostOutput<T, CostEvaluator::outputCount> with;
    if (!_f->evaluate(Xlocal, with.F)) {
      return false;
    }
    with.done(totalCost);
    return true;
  }

  bool accumulateNormalEquations(
      const T *Xlocal, SymmetricBandMatrixL<T> *JtJ,
      MDArray<T, 2> *minusJtF, T *totalCost) override {
    ADInputVector<T, CostEvaluator::inputCount> input(
        Xlocal);
    WithJacobianOutput<T, CostEvaluator::outputCount,
      CostEvaluator::inputCount> output;
    if (!_f->evaluate(input.X, output.F)) {
      return false;
    }
    output.done(this->inputRange.minv(), JtJ, minusJtF, totalCost);
    return true;
  }
private:
  std::shared_ptr<CostEvaluator> _f;
};

template <typename T>
struct IterationSummary {
  int iterationsCompleted;
  Eigen::Matrix<T, Eigen::Dynamic, 1> X;
};

template <typename T>
class Problem {
public:
  Problem(int expectedCostFunctionCount = -1) : _kd(0),
    _paramCount(0) {
    if (expectedCostFunctionCount > 0) {
      _costFunctions.reserve(expectedCostFunctionCount);
    }
  }

  template <typename CostEvaluator>
  std::shared_ptr<CostFunctionBase<T>> addCostFunction(Spani inputRange,
      const std::shared_ptr<CostEvaluator> &f) {
    std::shared_ptr<CostFunctionBase<T>> cost(
            new SharedCostFunction<CostEvaluator, T>(
                inputRange, f));
    CHECK(cost);
    return addCost(cost);
  }

  template <typename CostEvaluator>
  std::shared_ptr<CostFunctionBase<T>> addCostFunction(Spani inputRange,
      CostEvaluator *f) {
    return addCostFunction<CostEvaluator>(inputRange,
        std::shared_ptr<CostEvaluator>(f));
  }


  int kd() const {return _kd;}
  int paramCount() const {return _paramCount;}

  bool fillNormalEquations(
      const T *X,
      SymmetricBandMatrixL<T> *JtJ, MDArray<T, 2> *minusJtF,
      T *totalCost) const {
    *JtJ = SymmetricBandMatrixL<T>::zero(_paramCount, _kd);
    *minusJtF = MDArray<T, 2>(_paramCount, 1);
    minusJtF->setAll(T(0.0));
    *totalCost = T(0.0);
    for (auto &f: _costFunctions) {
      if (!f->accumulateNormalEquations(X + f->inputRange.minv(),
          JtJ, minusJtF, totalCost)) {
        LOG(ERROR) << "Failed to accumulate normal equations";
        return false;
      }
    }
    return true;
  }

  bool evaluate(const T *X, T *totalCost) const {
    *totalCost = T(0.0);
    for (const auto &f: _costFunctions) {
      if (!f->accumulateCost(X + f->inputRange.minv(), totalCost)) {
        return false;
      }
    }
    return true;
  }


  typedef std::function<void(IterationSummary<T>)> IterationCallback;

  void addIterationCallback(
      IterationCallback cb) {
    _callbacks.push_back(cb);
  }

  const std::vector<IterationCallback> &callbacks() const {
    return _callbacks;
  }

  void callIterationCallbacks(
      int iterationsCompleted,
      const Eigen::Matrix<T, Eigen::Dynamic, 1> &X) const {
    if (!_callbacks.empty()) {
      IterationSummary<T> summary{iterationsCompleted, X};
      for (auto cb: _callbacks) {
        cb(summary);
      }
    }
    for (auto f: _costFunctions) {
      f->callIterationCallbacks(iterationsCompleted, X.data());
    }
  }


  std::shared_ptr<CostFunctionBase<T>> addCost(std::shared_ptr<CostFunctionBase<T>> &cost) {
    _kd = std::max(_kd, cost->inputCount()-1);
    _paramCount = std::max(_paramCount, cost->inputRange.maxv());
    _costFunctions.push_back(cost);
    CHECK(cost);
    return cost;
  }
private:
  int _kd, _paramCount;
  std::vector<std::shared_ptr<CostFunctionBase<T> > > _costFunctions;
  std::vector<IterationCallback> _callbacks;

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
    PbsvFailed = 5,
    IterationsExceeded = 6,
    InnerStepFailed = 7
  };

  TerminationType type = None;
  int iterationsCompleted = 0;

  bool failure() const {
    return 3 <= static_cast<int>(type);
  }

  bool success() const {return !failure();}
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
    T elem = X(i, 0);
    maxv = std::max(T(maxv), T(abs(elem)));
  }
  return maxv;
}

// Implemented closely according to
// http://users.ics.forth.gr/~lourakis/levmar/levmar.pdf
template <typename T>
Results runLevMar(
    const Settings &settings,
    const Problem<T> &problem,
    Eigen::Matrix<T, Eigen::Dynamic, 1> *X) {
  Results results;
  T v = T(2.0);
  T mu = T(-1.0);

  SymmetricBandMatrixL<T> JtJ0;
  MDArray<T, 2> minusJtF0;
  problem.callIterationCallbacks(0, *X);
  for (int i = 0; i < settings.iters; i++) {
    if (1 <= settings.verbosity) {
      LOG(INFO) << "--------- LevMar Iteration " << i;
    }
    T currentCost = T(0.0);
    if (!problem.fillNormalEquations(X->data(), &JtJ0, &minusJtF0,
        &currentCost)) {
      results.type = Results::FullEvaluationFailed;
      LOG(ERROR) << "Full evaluation failed";
      return results;
    }
    if (3 <= settings.verbosity) {
      std::cout << "Right hand side: \n";
      std::cout << "  " << minusJtF0;
    }

    if (i == 0) {
      mu = settings.tau*getMaxDiagElement(JtJ0);
    }

    bool found = false;
    for (int j = 0; j < settings.subIters; j++) {
      if (2 <= settings.verbosity) {
        LOG(INFO) << "#### Inner iteration " << j;
      }
      auto JtJ = JtJ0.dup();
      auto minusJtF = minusJtF0.dup();

      if (2 <= settings.verbosity) {
        LOG(INFO) << "Damping: " << mu;
      }

      addDamping(mu, &JtJ);
      if (!Pbsv<T>::apply(&JtJ, &minusJtF)) {
        results.type = Results::PbsvFailed;
        LOG(ERROR) << "PBSV failed";
        return results;
      }

      T xNorm = sqrt(X->squaredNorm() + MakeConstant<T>::apply(1.0e-30));
      CHECK(isFinite(xNorm));
      auto eigenStep = wrapEigen1(minusJtF);
      if (eigenStep.norm() < settings.e2*xNorm) {
        if (1 <= settings.verbosity) {
          LOG(INFO) << "LevMar converged with sufficiently low step size";
        }
        results.type = Results::Converged;
        return results;
      }

      if (2 <= settings.verbosity) {
        LOG(INFO) << "Step size: " << eigenStep.norm();
      }
      Vec<T> xNew = *X + eigenStep;


      T newCost = T(0.0);
      if (!problem.evaluate(xNew.data(), &newCost)) {
        results.type = Results::ResidualEvaluationFailed;
        LOG(ERROR) << "Residual evaluation failed";
        return results;
      }

      // Is positive when improved:
      T improvement = currentCost - newCost;
      if (2 <= settings.verbosity) {
        LOG(INFO) << "Old residual squared norm: " << currentCost;
        LOG(INFO) << "New residual squared norm: " << newCost;
        LOG(INFO) << "Improvements: " << improvement;
      }

      auto eigenMinusJtF = wrapEigen1(minusJtF);
      T denom = eigenStep.dot(mu*eigenStep + eigenMinusJtF);
      T rho = improvement/denom;

      if (2 <= settings.verbosity) {
        LOG(INFO) << "RHO = " << rho;
      }

      if (!isFinite(rho)) {
        LOG(ERROR) << "Rho is not finite, cancel\n";
        results.type = Results::InnerStepFailed;
        return results;
      } else if (T(0.0) < rho) {
        if (2 <= settings.verbosity) {
          LOG(INFO) << "Accept the update";
        }
        *X = xNew;
        if (maxAbs(minusJtF) < settings.e1
            || sqrt(currentCost) < settings.e3) {
          if (1 <= settings.verbosity) {
            LOG(INFO) << "LevMar converged with low gradient or residual norm";
          }
          results.type = Results::Converged;
          return results;
        }
        mu *= acceptedUpdateFactor(rho);
        v = T(2.0);

        found = true;
        break;
      } else {
        if (2 <= settings.verbosity) {
          LOG(INFO) << "Reject the update";
        }
        mu *= v;
        v *= T(2.0);
      }
    }
    if (!found) {
      LOG(ERROR) << "Iterations exceeded";
      results.type = Results::IterationsExceeded;
      return results;
    }

    results.iterationsCompleted = i+1;
    problem.callIterationCallbacks(i+1, *X);
  }

  if (1 <= settings.verbosity) {
    LOG(INFO) << "Max iteration count " << settings.iters << " reached";
  }

  results.type = Results::MaxIterationsReached;
  return results;
}

}
}




#endif /* SERVER_MATH_BANDEDLEVMAR_H_ */
