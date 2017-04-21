/*
 * Resampler.cpp
 *
 *  Created on: May 20, 2016
 *      Author: jonas
 */

#include <server/math/SampleUtils.h>
#include <algorithm>
#include <server/common/ArrayBuilder.h>
#include <server/common/ArrayIO.h>
#include <server/common/logging.h>


namespace sail {
namespace SampleUtils {


Array<Endpoint> listEndpoints(const Array<TimeStamp> &samples,
    Duration<double> margin) {
  int n = samples.size();
  Array<Endpoint> eps(2*n);
  for (int i = 0; i < n; i++) {
    auto x = samples[i];
    auto at = 2*i;
    // Here we are essentially dilating every sample, by replacing it
    // by an interval of width 2*period
    eps[at + 0] = Endpoint{x - margin, true};
    eps[at + 1] = Endpoint{x + margin, false};
  }
  std::sort(eps.begin(), eps.end());
  return eps;
}

void addNewSamples(ArrayBuilder<TimeStamp> *dst, TimeStamp from,
    TimeStamp to, Duration<double> period) {
  for (auto i = from; i <= to; i = i + period) {
    dst->add(i);
  }
}

Array<Span<TimeStamp>> makeContinuousSpans(
    const Array<Endpoint> &eps,
    Duration<double> cropMargin = 0.0_s) {
  int sum = 0;
  ArrayBuilder<Span<TimeStamp>> spans;
  TimeStamp start;
  for (auto ep: eps) {
    sum += (ep.rising? 1 : -1);
    if (sum == 1 && ep.rising) {
      start = ep.pos;
    } else if (sum == 0 && !ep.rising) {
      auto from = start + cropMargin;
      auto to = ep.pos - cropMargin;
      if (from <= to) {
        spans.add(Span<TimeStamp>(from, to));
      }
    }
  }
  return spans.get();
}

Array<Span<TimeStamp>> makeContinuousSpansWithMargin(
    const Array<TimeStamp> &timeSamples,
    Duration<double> margin) {
  return makeContinuousSpans(listEndpoints(timeSamples, margin), 0.0_s);
}

Array<Span<TimeStamp>> makeContinuousSpans(
    const Array<TimeStamp> &timeSamples,
    Duration<double> margin) {
  return makeContinuousSpans(listEndpoints(timeSamples, margin), margin);
}

Array<TimeStamp> makeNewSamplesFromEndpoints(const Array<Endpoint> &eps,
    Duration<double> period) {
  int sum = 0;
  TimeStamp start;
  ArrayBuilder<TimeStamp> newSamples;
  for (auto span: makeContinuousSpans(eps)) {
    addNewSamples(&newSamples,
        span.minv()+period,
        span.maxv()-period, period);
  }
  return newSamples.get();
}

Array<TimeStamp> resample(const Array<TimeStamp> &irregularSamples,
    Duration<double> newSamplingPeriod) {
  return makeNewSamplesFromEndpoints(
      listEndpoints(irregularSamples, newSamplingPeriod),
      newSamplingPeriod);
}

bool isCovered(TimeStamp t, const Array<Span<TimeStamp>>& spans) {
  if (spans.empty()) {
    return false;
  } else if (spans.size() == 1) {
    return spans.first().contains(t);
  } else {
    auto middle = spans.size()/2;
    return isCovered(t, spans[middle-1].maxv() < t?
        spans.sliceFrom(middle) : spans.sliceTo(middle));
  }
}

Array<TimeStamp> getTimesOfType(
    const Array<TimedValue<bool>>& good,
    bool type) {
  ArrayBuilder<TimeStamp> dst(good.size());
  for (auto x: good) {
    if (x.value == type) {
      dst.add(x.time);
    }
  }
  return dst.get();
}


Array<Span<TimeStamp>> getBadSpans(const Array<TimedValue<bool>>& good,
    Duration<double> badMargin) {
  return makeContinuousSpansWithMargin(
      getTimesOfType(good, false), badMargin);
}

Array<TimeStamp> filterGoodTimes(
    const Array<TimedValue<bool>>& good,
    const Array<Span<TimeStamp>>& badSpans) {
  ArrayBuilder<TimeStamp> dst(good.size());
  for (auto x: good) {
    if (x.value && !isCovered(x.time, badSpans)) {
      dst.add(x.time);
    }
  }
  return dst.get();
}

Array<Span<TimeStamp>> makeGoodSpans(
    const Array<TimedValue<bool>>& good,
    Duration<double> goodMargin, Duration<double> badMargin) {
  CHECK(std::is_sorted(good.begin(), good.end()));
  auto badSpans = getBadSpans(good, badMargin);
  std::cout << "Number of bad spans: " << badSpans.size() << std::endl;
  for (auto bs: badSpans) {
    std::cout << "  Bad span from "
        << bs.minv().toIso8601String() << " to "
        << bs.maxv().toIso8601String() << std::endl;
  }
  auto goodTimes = filterGoodTimes(good, badSpans);
  std::cout << "Number of good times: " << goodTimes.size() << "/" << good.size() << std::endl;
  auto maxGap = 0.0_s;
  for (int i = 0; i < goodTimes.size()-1; i++) {
    maxGap = std::max(maxGap, goodTimes[i+1] - goodTimes[i]);
  }
  std::cout << "Max gap of " << maxGap.str() << std::endl;
  std::cout << "First good time:  " << goodTimes.first().toIso8601String() << std::endl;
  std::cout << "Last good time: " << goodTimes.last().toIso8601String() << std::endl;
  return makeContinuousSpans(goodTimes, goodMargin);
}


}
}

