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

namespace sail {
namespace BandedLevMar {

template <typename T>
class CostFunctionBase {
public:
  virtual int inputCount() const = 0;
  virtual int outputCount() const = 0;
  virtual Spani inputRange() const = 0;
  virtual bool evaluate(const T *X, T *outLocal) = 0;
  virtual bool evaluate(const T *X, T *Y, T *JcolMajor) = 0;
};

template <typename CostEvaluator, typename T>
class UniqueCostFunction : public CostFunctionBase<T> {
public:
  typedef ceres::Jet<T, 1> ADType;

  UniqueCostFunction(
      const Spani &inputRange,
      CostEvaluator *f) :
        _inputRange(inputRange),
        _inputCount(f->inputCount),
        _outputCount(f->outputCount),
        _f(f) {
    CHECK(_inputRange.width() == _inputCount);
    _adX.resize(_inputCount);
    _adY.resize(_outputCount);
  }

  Spani inputRange() const override {
    return _inputRange;
  }

  int outputCount() const override {
    return _outputCount;
  }

  int inputCount() const override {
    return _inputCount;
  }

  bool evaluate(const T *X, T *outLocal) override {
    return _f->evaluate(X + _inputRange.minv(), outLocal);
  }

  bool evaluate(const T *X, T *Y, T *J) override {
    for (int i = 0; i < _inputCount; i++) {
      _adX[i] = ADType(X[i]);
    }
    for (int i = 0; i < _inputCount; i++) {
      _adX[i].v[0] = 1.0;
      if (!_f->template evaluate<ceres::Jet<T, 1> >(_adX.data(), _adY.data())) {
        return false;
      }
      _adX[i].v[0] = 0.0;

      for (int j = 0; j < _outputCount; j++) {
        Y[j] = _adY[j].a;
        J[j] = _adY[j].v[0];
      }
      J += _outputCount;
    }
    return true;
  }
private:
  std::vector<ADType> _adX, _adY;
  Spani _inputRange;
  int _inputCount, _outputCount;
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
