/*
 * TimedValueIntegrator.cpp
 *
 *  Created on: Apr 15, 2016
 *      Author: jonas
 */

#include <server/nautical/filters/TimedValueIntegrator.h>
#include <server/common/IntervalUtils.h>

namespace sail {

Arrayd computeBounds(const Arrayd &localTimes) {
  int n = localTimes.size();
  if (n == 0) {
    return Arrayd();
  } else if (n == 1) {
    return Arrayd{0.0};
  } else {
    double leftmostWidth = localTimes[1] - localTimes[0];
    Arrayd widths(n);
    widths[0] = leftmostWidth;
    widths[n-1] = localTimes[n-1] - localTimes[n-2];
    for (int i = 1; i < n-1; i++) {
      widths[i] = 0.5*((localTimes[i] - localTimes[i-1])
          + (localTimes[i+1] - localTimes[i]));
    }
    Arrayd bounds(n + 1);
    bounds[0] = -0.5*leftmostWidth;
    for (int i = 0; i < n; i++) {
      bounds[i+1] = bounds[i] + widths[i];
    }
    return bounds;
  }
}

int computeBin(const Arrayd &bounds, double x) {
  return findIntervalIndex(bounds.begin(), bounds.end(), x);
}


}
