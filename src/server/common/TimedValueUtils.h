/*
 * TimedValueUtils.h
 *
 *  Created on: 10 Nov 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TIMEDVALUEUTILS_H_
#define SERVER_COMMON_TIMEDVALUEUTILS_H_

#include <server/common/TimeStamp.h>
#include <server/common/TimedValue.h>
#include <server/common/Array.h>
#include <server/common/Span.h>

namespace sail {

template <typename T>
Array<TimeStamp> getTimes(const Array<TimedValue<T>> &src) {
  int n = src.size();
  Array<TimeStamp> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = src[i].time;
  }
  return dst;
}

Array<int> listAllBounds(
    const Array<TimeStamp> &times,
    Duration<double> dur);

Array<Span<TimeStamp>> listTimeSpans(
    const Array<TimeStamp> &times,
    const Duration<double> dur,
    bool includeEmpty);

Array<int> getTimeSpanPerTimeStamp(
    const Array<Span<TimeStamp>> &timeSpans,
    const Array<TimeStamp> &timeStamps);

Array<TimeStamp> merge(
    const Array<TimeStamp> &a,
    const Array<TimeStamp> &b);

Array<int> findNearestTimePerTime(
    const Array<TimeStamp> &src,
    const Array<TimeStamp> &dst);

} /* namespace sail */

#endif /* SERVER_COMMON_TIMEDVALUEUTILS_H_ */
