/*
 * DiscreteOutlierFilter.h
 *
 *  Created on: 4 Nov 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_DISCRETEOUTLIERFILTER_H_
#define SERVER_COMMON_DISCRETEOUTLIERFILTER_H_

#include <server/common/Array.h>

namespace sail {
namespace DiscreteOutlierFilter {

struct Settings {
  double threshold = 1.0e6;
  Array<int> backSteps = Array<int>{1, 2, 4, 8, 16, 32, 64, 128};
};

Array<double> computeOutlierMaskFromPairwiseCosts(
    const Array<double> &pairwiseCosts, const Settings &settings);


// Note: don't forget to make sure the cost is positive,
//
template <typename Iterator, typename T = typename Iterator::type>
Array<bool> computeOutlierMask(Iterator begin, Iterator end,
    std::function<double(TimedValue<T>, TimedValue<T>)> cost,
    const Settings &settings) {
  int n = std::distance(begin, end);
  if (n <= 1) {
    return Array<bool>::fill(n, true);
  }
  auto butLast = end-1;
  ArrayBuilder<double> costs(n-1);
  for (auto iter = begin; iter != butLast; iter++) {
    costs.add(cost(*iter, *(iter + 1)));
  }
  return computeOutlierMaskFromPairwiseCosts(
      costs.get(), settings);
}

}
}

#endif /* SERVER_COMMON_DISCRETEOUTLIERFILTER_H_ */
