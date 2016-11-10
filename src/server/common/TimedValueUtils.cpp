/*
 * TimedValueUtils.cpp
 *
 *  Created on: 10 Nov 2016
 *      Author: jonas
 */

#include "TimedValueUtils.h"
#include <server/common/ArrayBuilder.h>

namespace sail {

Array<int> listAllBounds(const Array<TimeStamp> &times,
    Duration<double> thresh) {
  ArrayBuilder<int> gaps;
  gaps.add(0);

  int n = times.size();
  for (int i = 0; i < times.size() - 1; i++) {
    if (times[i+1] - times[i] > thresh) {
      gaps.add(i+1);
    }
  }

  if (gaps.last() != n) {
    gaps.add(n);
  }
  return gaps.get();
}

Array<Span<TimeStamp>> listTimeSpans(
    const Array<TimeStamp> &times,
    const Array<int> &bds,
    bool includeEmpty) {
  ArrayBuilder<Span<TimeStamp>> dst;
  for (int i = 0; i < bds.size()-1; i++) {
    int from = bds[i];
    int to = bds[i+1]-1;
    if (includeEmpty || from < to) {
      dst.add(Span<TimeStamp>(times[from], times[to]));
    }
  }
  return dst.get();
}

Array<Span<TimeStamp>> listTimeSpans(
    const Array<TimeStamp> &times,
    const Duration<double> dur,
    bool includeEmpty) {
  auto bds = listAllBounds(times, dur);
  return listTimeSpans(times, bds, includeEmpty);
}

} /* namespace sail */
