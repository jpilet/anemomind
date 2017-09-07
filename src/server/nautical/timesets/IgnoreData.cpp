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
    const std::vector<TimeSetInterval>& intervals) {
  if (intervals.empty()) {
    return false;
  } else {
    while (*lastPos < intervals.size()-1
        && t < intervals[*lastPos].span.minv()) {
      (*lastPos)++;
    }
    while (*lastPos > 0 && intervals[*lastPos].span.maxv() < t) {
      (*lastPos)--;
    }
    return intervals[*lastPos].span.contains(t);
  }
}

struct IgnoreTransducer {
  std::vector<TimeSetInterval> intervals;

  template <typename Acc, typename T>
  Step<Acc, TimedValue<T>> operator()(
      const Step<Acc, TimedValue<T>>& s) const {
    auto at = std::make_shared<int>(0);
    auto ivals = intervals;

    // Construct an inner helper transducer based on T.
    return filter([ivals, at](const TimedValue<T>& x) {
      auto keep = !coveredByInterval(at.get(), x.time, ivals);
      std::cout << "Keep it at time " << x.time.toString() << "? "
          << keep << std::endl;
      return keep;
    })(s);
  }
};

std::function<
  std::shared_ptr<DispatchData>(std::shared_ptr<DispatchData>)>
    ignoreDispatchData(
        const Array<TimeSetInterval>& allIntervals,
        const std::set<std::string>& typesOfInterest) {

  IgnoreTransducer t;

  // Only keep the intervals that we are interested in
  transduceIntoColl(
      filter([&typesOfInterest](const TimeSetInterval& t) {
        return 0 < typesOfInterest.count(t.type);
      }),
      &(t.intervals),
      allIntervals);

  return [t](const std::shared_ptr<DispatchData>& src) {
    return transduceDispatchData<IgnoreTransducer>(nullptr, src, t);
  };
}

} /* namespace sail */
