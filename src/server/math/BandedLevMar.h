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

template <typename CostFunctor, typename T>
class CostFunction {
public:
  CostFunction(
      const Spani &inputRange,
      int residualCount, CostFunctor *f) :
        _inputRange(inputRange),
        _residualCount(residualCount),
        _f(f) {}
private:
  Spani _inputRange;
  int _residualCount;
  std::unique_ptr<CostFunctor> _f;
};

template <typename T>
class Problem {
public:
private:
};

}
}




#endif /* SERVER_MATH_BANDEDLEVMAR_H_ */
