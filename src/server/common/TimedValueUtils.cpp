/*
 * TimedValueUtils.cpp
 *
 *  Created on: 10 Nov 2016
 *      Author: jonas
 */

#include "TimedValueUtils.h"

namespace sail {

Array<int> listAllBounds(const Array<TimeStamp> &times,
    Duration<double> gap) {
  ArrayBuilder<int> gaps;
  gaps.add(0);

  for (int i = 0; i < times.size() - 1; i++) {
    if (times[i+1] - times[i] > dur) {
      gaps.add(i+1);
    }
  }

  if (gaps.last() != gaps.size()) {
    gaps.add(gaps.size());
  }
  return gaps.get();
}

} /* namespace sail */
