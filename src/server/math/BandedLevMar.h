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
  virtual void evaluateResiduals(const T *X, const T *outLocal) const = 0;
};

template <typename CostEvaluator, typename T>
class UniqueCostFunction {
public:
  UniqueCostFunction(
      const Spani &inputRange,
      const std::unique_ptr<CostEvaluator> &f) :
        _inputRange(inputRange),
        _inputCount(f->inputCount()),
        _outputCount(f->outputCount()),
        _f(f) {
    CHECK(_inputRange.width() == _inputCount);
  }

  Spani inputRange() const {
    return _inputRange;
  }

  int outputCount() const {
    return _outputCount;
  }

  int inputCount() const {
    return _inputCount;
  }

  void evaluateResiduals(const T *X, const T *outLocal) const {
    _f->evaluate(X + _inputRange.minv(), outLocal);
  }
private:
  Spani _inputRange;
  int _inputCount, _outputCount;
  std::unique_ptr<CostEvaluator> _f;
};

template <typename T>
class Problem {
public:
  Problem() {}

  Problem(int expectedCostFunctionCount) {
    _costFunctions.reserve(expectedCostFunctionCount);
  }

  template <typename CostEvaluator>
  void addCostFunction(Spani inputRange, int residualCount,
      CostEvaluator *f) {
    std::unique_ptr<CostFunctionBase<T>> cost(
        new UniqueCostFunction<CostEvaluator, T>(inputRange, residualCount, f));
    _costFunctions.push_back(std::move(cost));
  }
private:
  std::vector<std::unique_ptr<CostFunctionBase<T> > > _costFunctions;
};

}
}




#endif /* SERVER_MATH_BANDEDLEVMAR_H_ */
