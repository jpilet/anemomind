/*
 * IgnoreData.cpp
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#include "IgnoreData.h"
#include <server/common/TimeStamp.h>
#include <device/anemobox/TransduceDispatcher.h>

namespace sail {

bool coveredByInterval(
    int* lastPos, TimeStamp t,
    const std::vector<Span<TimeStamp>>& intervals) {
  if (intervals.empty()) {
    return false;
  } else {
    while (*lastPos < intervals.size()-1
        && t < intervals[*lastPos].minv()) {
      (*lastPos)++;
    }
    while (*lastPos > 0 && intervals[*lastPos].maxv() < t) {
      (*lastPos)--;
    }
    return intervals[*lastPos].contains(t);
  }
}

struct IgnoreFactory {
  std::vector<Span<TimeStamp>> intervals;

  template <DataCode code, typename T>
  Filter<std::function<bool(TimedValue<T>)>> make() const {
    auto at = std::make_shared<int>(0);
    auto ivals = intervals;
    return Filter<std::function<bool(TimedValue<T>)>>(
        [at, ivals](const TimedValue<T>& v) {
      return !coveredByInterval(at.get(), v.time, ivals);
    });
  }
};

std::function<bool(TimeSetInterval)> timeSetIntervalOfType(
    const std::set<std::string>& types) {
  return [types](const TimeSetInterval& v) {
    return 0 < types.count(v.type);
  };
}

Span<TimeStamp> getTimeSpan(const TimeSetInterval& x) {
  return x.span;
}

std::function<
  std::shared_ptr<DispatchData>(std::shared_ptr<DispatchData>)>
    ignoreDispatchData(
        Clock* clk,
        const Array<TimeSetInterval>& allIntervals,
        const std::set<std::string>& typesOfInterest) {
  IgnoreFactory f;
  auto T = composeTransducers(
            filter(timeSetIntervalOfType(typesOfInterest)),
            map(&getTimeSpan));
  transduceIntoColl(
      T, &(f.intervals), allIntervals);
  return [clk, f](const std::shared_ptr<DispatchData>& src) {
    return transduceDispatchData<IgnoreFactory>(clk, src, f);
  };
}

} /* namespace sail */
