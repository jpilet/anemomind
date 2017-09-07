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


std::function<bool(TimeSetInterval)> timeSetIntervalOfType(
    const std::set<std::string>& types) {
  return [types](const TimeSetInterval& v) {
    return 0 < types.count(v.type);
  };
}

Span<TimeStamp> getTimeSpan(const TimeSetInterval& x) {
  return x.span;
}

struct IgnoreTransducer {
  std::vector<Span<TimeStamp>> intervals;

  template <typename Acc, typename T>
  Step<Acc, TimedValue<T>> operator()(
      const Step<Acc, TimedValue<T>>& s) const {
    auto at = std::make_shared<int>(0);
    auto ivals = intervals;
    auto t = filter([ivals, at](const TimedValue<T>& x) {
      return !coveredByInterval(at.get(), x.time, ivals);
    });
    return t(s);
  }
};

std::function<
  std::shared_ptr<DispatchData>(std::shared_ptr<DispatchData>)>
    ignoreDispatchData(
        Clock* clk,
        const Array<TimeSetInterval>& allIntervals,
        const std::set<std::string>& typesOfInterest) {
  auto T = composeTransducers(
            filter(timeSetIntervalOfType(typesOfInterest)),
            map(&getTimeSpan));
  IgnoreTransducer t;
  transduceIntoColl(
      T, &(t.intervals), allIntervals);

  return [clk, t](const std::shared_ptr<DispatchData>& src) {
    return transduceDispatchData<IgnoreTransducer>(clk, src, t);
  };
}

} /* namespace sail */
