/*
 * TimedValueUtils.cpp
 *
 *  Created on: 10 Nov 2016
 *      Author: jonas
 */

#include "TimedValueUtils.h"
#include <server/common/ArrayBuilder.h>
#include <server/common/logging.h>

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

Array<int> getTimeSpanPerTimeStamp(
    const Array<Span<TimeStamp>> &timeSpans,
    const Array<TimeStamp> &timeStamps) {
  CHECK(std::is_sorted(timeStamps.begin(), timeStamps.end()));

  int currentSpanIndex = 0;
  int n = timeStamps.size();
  Array<int> dst = Array<int>::fill(n, -1);
  for (int i = 0; i < n; i++) {
    auto x = timeStamps[i];
    while (currentSpanIndex < timeSpans.size() &&
        timeSpans[currentSpanIndex].maxv() < x) {
      currentSpanIndex++;
    }
    if (timeSpans.size() <= currentSpanIndex) {
      break;
    }
    if (timeSpans[currentSpanIndex].minv() <= x) {
      dst[i] = currentSpanIndex;
    }
  }
  return dst;
}

} /* namespace sail */
