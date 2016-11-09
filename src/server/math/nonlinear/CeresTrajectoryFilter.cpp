/*
 * CeresTrajectoryFilter.cpp
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#include <server/math/nonlinear/CeresTrajectoryFilter.h>

namespace sail {
namespace CeresTrajectoryFilter {

int findIntervalWithTolerance(const AbstractArray<TimeStamp> &times, TimeStamp t) {
  Duration<double> maxdur = Duration<double>::seconds(1.0);
  int intervalCount = times.size() - 1;
  int i = findIntervalIndex(times.begin(), times.end(), t);
  if (i < 0) {
    if (times.first() - t < maxdur) {
      return 0;
    } else {
      return -1;
    }
  } else if (!(i <intervalCount)) {
    if (t - times.last() < maxdur) {
      return intervalCount-1;
    } else {
      return -1;
    }
  } else {
    return i;
  }
}


}
}



