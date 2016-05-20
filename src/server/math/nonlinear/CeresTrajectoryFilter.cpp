/*
 * CeresTrajectoryFilter.cpp
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#include <server/math/nonlinear/CeresTrajectoryFilter.h>

namespace sail {
namespace CeresTrajectoryFilter {

int getIntervalCount(int sampleCount) {
  return sampleCount - 1;
}

double getRightmost(const Arrayd &samples, int intervalIndex) {
  int intervalCount = getIntervalCount(samples.size()-1);
  if (intervalIndex < 0) {
    return samples.first();
  } else if (intervalIndex < intervalCount) {
    return samples[intervalIndex+1];
  }
  return samples.last();
}

int moveIntervalIndexForward(const Arrayd &samples, int intervalIndex, double t) {
  assert(0 <= intervalIndex);
  int intervalCount = getIntervalCount(samples.size());
  int lastIntervalIndex = intervalCount - 1;
  while (intervalIndex < lastIntervalIndex) {
    if (getRightmost(samples, intervalIndex) < t) {
      intervalIndex++;
    } else {
      return intervalIndex;
    }
  }
  return intervalIndex;
}

}
}



