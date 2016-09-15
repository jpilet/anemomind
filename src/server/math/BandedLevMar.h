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

namespace sail {
namespace BandedLevMar {

class SampleCostEvaluator {
  int inputCount() const {return 2;}
  int outputCount() const {return 2;}

  template <typename T>
  bool evaluate(const T *input, T *output) const {
    output[0] = input[0] - 3.4;
    output[1] = input[1] - 5.6;
  };
};

template <typename T>
class CostFunctionBase {
public:
  virtual int inputCount() const = 0;
  virtual int outputCount() const = 0;
  virtual Spani inputRange() const = 0;
  virtual bool evaluateResiduals(const T *X, T *outLocal) const = 0;
};

template <typename CostEvaluator, typename T>
class UniqueCostFunction : public CostFunctionBase<T> {
public:
  UniqueCostFunction(
      const Spani &inputRange,
      CostEvaluator *f) :
        _inputRange(inputRange),
        _inputCount(f->inputCount()),
        _outputCount(f->outputCount()),
        _f(f) {
    CHECK(_inputRange.width() == _inputCount);
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

  bool evaluateResiduals(const T *X, T *outLocal) const override {
    return _f->evaluate(X + _inputRange.minv(), outLocal);
  }
private:
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
