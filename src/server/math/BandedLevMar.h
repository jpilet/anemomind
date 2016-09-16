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
  Problem(int expectedCostFunctionCount = -1) : _bandWidth(0),
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

  int bandWidth() const {return _bandWidth;}
  int paramCount() const {return _paramCount;}
  int residualCount() const {return _residualCount;}

  bool fillNormalEquations(
      const T *X,
      SymmetricBandMatrixL<T> *JtJ, MDArray<T, 2> *minusJtF) {
    *JtJ = SymmetricBandMatrixL<T>::zero(_paramCount, _bandWidth);
    *minusJtF = MDArray<T, 2>(_paramCount, 1);
    minusJtF->setAll(T(0.0));
    for (auto &f: _costFunctions) {
      if (!f->accumulateNormalEquations(X, JtJ, minusJtF)) {
        return false;
      }
    }
    return true;
  }
private:
  int _bandWidth, _paramCount, _residualCount;
  std::vector<std::unique_ptr<CostFunctionBase<T> > > _costFunctions;

  void addCost(std::unique_ptr<CostFunctionBase<T>> &cost) {
    _bandWidth = std::max(_bandWidth, cost->inputCount()-1);
    _paramCount = std::max(_paramCount, cost->inputRange().maxv());
    _residualCount += cost->outputCount();
    _costFunctions.push_back(std::move(cost));
  }
};

}
}




#endif /* SERVER_MATH_BANDEDLEVMAR_H_ */
