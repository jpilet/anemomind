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
  virtual bool accumulateCost(const T *X, T *totalCost) = 0;
  virtual bool accumulateNormalEquations(
      const T *X,
      SymmetricBandMatrixL<T> *JtJ,
      MDArray<T, 2> *minusJtF,
      T *totalCost) = 0;
};

template <typename CostEvaluator, typename T>
class SharedCostFunction : public CostFunctionBase<T> {
public:
  static const int N = CostEvaluator::inputCount;
  typedef ceres::Jet<T, N> ADType;

  SharedCostFunction(
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

  bool accumulateCost(const T *X, T *totalCost) override {
    T residuals[CostEvaluator::outputCount];
    if (!_f->evaluate(X + _inputRange.minv(), residuals)) {
      return false;
    }
    for (int i = 0; i < CostEvaluator::outputCount; i++) {
      T f = residuals[i];
      *totalCost += f*f;
    }
    return true;
  }

  bool evaluate(const T *X, T *Y, T *J) {
    ADType _adX[CostEvaluator::inputCount];
    ADType _adY[CostEvaluator::outputCount];
    for (int i = 0; i < CostEvaluator::inputCount; i++) {
      auto ad = ADType(X[i]);
      ad.v[i] = T(1.0);
      _adX[i] = ad;
    }

    if (!_f->template evaluate<ADType>(_adX, _adY)) {
      return false;
    }

    for (int i = 0; i < CostEvaluator::outputCount; i++) {
      auto y = _adY[i];
      Y[i] = y.a;
      auto Jsub = J + i;
      for (int j = 0; j < CostEvaluator::inputCount; j++) {
        Jsub[j*CostEvaluator::outputCount] = y.v[j];
      }
    }
    return true;
  }

  bool accumulateNormalEquations(
      const T *X, SymmetricBandMatrixL<T> *JtJ,
      MDArray<T, 2> *minusJtF, T *totalCost) override {
    T F[CostEvaluator::outputCount];
    T J[CostEvaluator::outputCount*CostEvaluator::inputCount];
    if (!evaluate(X, F, J)) {
      return false;
    }

    for (int i = 0; i < CostEvaluator::outputCount; i++) {
      T f = F[i];
      *totalCost += f*f;
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
      }
    }
    return true;
  }
private:
  Spani _inputRange;
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
    _paramCount(0), _residualCount(0) {
    if (expectedCostFunctionCount > 0) {
      _costFunctions.reserve(expectedCostFunctionCount);
    }
  }

  template <typename CostEvaluator>
  void addCostFunction(Spani inputRange,
      CostEvaluator *f) {
    std::shared_ptr<CostFunctionBase<T>> cost(
            new SharedCostFunction<CostEvaluator, T>(inputRange, f));
    addCost(cost);
  }

  int kd() const {return _kd;}
  int paramCount() const {return _paramCount;}
  int residualCount() const {return _residualCount;}

  bool fillNormalEquations(
      const T *X,
      SymmetricBandMatrixL<T> *JtJ, MDArray<T, 2> *minusJtF,
      T *totalCost) const {
    *JtJ = SymmetricBandMatrixL<T>::zero(_paramCount, _kd);
    *minusJtF = MDArray<T, 2>(_paramCount, 1);
    minusJtF->setAll(T(0.0));
    *totalCost = T(0.0);
    int counter = 0;
    for (auto &f: _costFunctions) {
      if (!f->accumulateNormalEquations(f->inputRange().minv() + X,
          JtJ, minusJtF, totalCost)) {
        LOG(ERROR) << "Failed to accumulate normal equations";
        return false;
      }
      counter++;
    }
    return true;
  }

  bool evaluate(const T *X, T *totalCost) const {
    *totalCost = T(0.0);
    for (const auto &f: _costFunctions) {
      if (!f->accumulateCost(X, totalCost)) {
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

  void addCost(std::shared_ptr<CostFunctionBase<T>> &cost) {
    _kd = std::max(_kd, cost->inputCount()-1);
    _paramCount = std::max(_paramCount, cost->inputRange().maxv());
    _residualCount += cost->outputCount();
    _costFunctions.push_back(std::move(cost));
  }
private:
  int _kd, _paramCount, _residualCount;
  std::vector<std::shared_ptr<CostFunctionBase<T> > > _costFunctions;
  std::vector<IterationCallback> _callbacks;

};

template <typename T>
struct GemanMcClureLoss {
  T operator()(T x) const {
    T x2 = x*x;
    return x2/(x2 + T(1.0));
  }
};

template <typename T, typename LossFunction>
class RobustCost {
public:
  RobustCost(const shared_ptr<CostFunctionBase<T> > &src,
      const LossFunction &loss,
      T *median) : _src(src), _loss(loss), _median(median) {}
private:
  shared_ptr<CostFunctionBase<T> > _src;
  LossFunction _loss;
  T *_median;
};

template <typename T, typename LossFunction>
class MeasurementGroup {
public:
  typedef MeasurementGroup<T, LossFunction> ThisType;

  MeasurementGroup(
      Problem<T> *dstProblem, const LossFunction &loss) :
    _dstProblem(dstProblem), _loss(loss),
    _medianResidual(T(-1.0)) {
    dstProblem->addIterationCallback([this](
        const IterationSummary<T> &summary) {
      this->update(summary);
    });
  }

  template <typename CostEvaluator>
    void addCostFunction(Spani inputRange,
        CostEvaluator *f) {
    _indices.push_back(_costs.size());
    std::shared_ptr<CostFunctionBase<T>> hiddenCost(
            new SharedCostFunction<CostEvaluator, T>(inputRange, f));
    _costs.push_back(hiddenCost);
    _dstProblem->addCost(
        shared_ptr<CostFunctionBase<T> >(new RobustCost<T, LossFunction>(
            hiddenCost, _loss, &_medianResidual)));
  }
private:
  void update(const IterationSummary<T> &summary) {

  }

  MeasurementGroup(const ThisType &other) = delete;
  ThisType &operator=(const ThisType &other) = delete;
  Problem<T> *_dstProblem;
  LossFunction _loss;
  T _medianResidual;
  std::vector<T> _residuals;
  std::vector<std::shared_ptr<CostFunctionBase<T> > > _costs;
  std::vector<int> _indices;
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
    IterationsExceeded = 6
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

template <typename T>
void callIterationCallbacks(
    int iterationsCompleted,
    const Vec<T> &X,
    const std::vector<std::function<void(IterationSummary<T>)>> &callbacks) {
  if (!callbacks.empty()) {
    IterationSummary<T> summary{iterationsCompleted, X};
    for (auto cb: callbacks) {
      cb(summary);
    }
  }
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
  callIterationCallbacks<T>(0, *X, problem.callbacks());
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

      T xNorm = X->norm();
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

      if (T(0.0) < rho) {
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
    callIterationCallbacks<T>(i+1, *X, problem.callbacks());
  }

  if (1 <= settings.verbosity) {
    LOG(INFO) << "Max iteration count reached";
  }

  results.type = Results::MaxIterationsReached;
  return results;
}

}
}




#endif /* SERVER_MATH_BANDEDLEVMAR_H_ */
