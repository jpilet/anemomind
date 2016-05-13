/*
 * MotionsFromPairs.h
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_FILTERS_MOTIONSFROMPAIRS_H_
#define SERVER_NAUTICAL_FILTERS_MOTIONSFROMPAIRS_H_

#include <server/common/TimedValuePairs.h>

namespace sail {

// Usually, we have a channel for gps bearings and another channels for gps speeds.
// We might need to merge these two channels into a single set of timed horizontal motions.
// This code does that. But it can also be used with other such angle-velocity pairs.
template <typename TimedVelocityIterator, typename TimedAngleIterator>
Array<TimedValue<HorizontalMotion<double> > > makeMotionsFromVelocityAnglePairs(
    TimedVelocityIterator velBegin, TimedVelocityIterator velEnd,
    TimedAngleIterator angleBegin, TimedAngleIterator angleEnd,
    Duration<double> maxGap) {
  auto pairs = TimedValuePairs::makeTimedValuePairs(
      velBegin, velEnd, angleBegin, angleEnd);
  int n = pairs.size();
  ArrayBuilder<TimedValue<HorizontalMotion<double> > > dst(n);
  for (auto p: pairs) {
    auto diff = p.second.time - p.first.time;
    if (fabs(diff) < maxGap) {
      auto m = HorizontalMotion<double>::polar(p.first.value, p.second.value);
      auto t = p.first.time + 0.5*diff;
      dst.add(TimedValue<HorizontalMotion<double> >(t, m));
    }
  }
  return dst.get();
}


}



#endif /* SERVER_NAUTICAL_FILTERS_MOTIONSFROMPAIRS_H_ */
