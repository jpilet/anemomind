/*
 * Resampler.cpp
 *
 *  Created on: May 20, 2016
 *      Author: jonas
 */

#include <server/math/Resampler.h>
#include <algorithm>
#include <server/common/ArrayBuilder.h>


namespace sail {
namespace Resampler {


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

Array<Span<TimeStamp>> makeContinuousSpans(const Array<Endpoint> &eps) {
  int sum = 0;
  ArrayBuilder<Span<TimeStamp>> spans;
  TimeStamp start;
  for (auto ep: eps) {
    sum += (ep.rising? 1 : -1);
    if (sum == 1 && ep.rising) {
      start = ep.pos;
    } else if (sum == 0 && !ep.rising) {
      spans.add(Span<TimeStamp>(start, ep.pos));
    }
  }
  return spans.get();
}

Array<Span<TimeStamp>> makeContinuousSpans(
    const Array<TimeStamp> &timeSamples,
    Duration<double> margin) {
  return makeContinuousSpans(listEndpoints(timeSamples, margin));
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

}
}

