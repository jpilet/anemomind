/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/Sampling.h>

namespace sail {

Sampling::Sampling(int count, double lower, double upper) : sampleCount(count),
    _indexToX(0, count, lower, upper), _xToIndex(lower, upper, 0, count) {}

Sampling::Weights Sampling::represent(double x) const {
  auto index = _xToIndex(x);
  int lowerIndex = int(floor(index));
  double upperWeight = (index - lowerIndex);
  double lowerWeight = 1.0 - upperWeight;
  return Weights{lowerIndex, lowerWeight, upperWeight};
}

} /* namespace sail */
