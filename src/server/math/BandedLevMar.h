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

namespace sail {
namespace BandedLevMar {

class SampleCostEvaluator {
  template <typename T>
  bool evaluate(int inputDim, const T *input,
                int outputDim, T *output) const {
    return true;
  };
};

template <typename CostEvaluator, typename T>
class CostFunction {
public:
  CostFunction(
      const Spani &inputRange,
      int residualCount
      const std::shared_ptr<CostEvaluator> &f) :
        _inputRange(inputRange),
        _residualCount(residualCount),
        _f(f) {}
private:
  Spani _inputRange;
  int _residualCount;
  std::shared_ptr<CostEvalutor> _f;
};

template <typename T>
class Problem {
public:
private:
};

}
}




#endif /* SERVER_MATH_BANDEDLEVMAR_H_ */
