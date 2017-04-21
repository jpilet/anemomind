/*
 * Resampler.h
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_RESAMPLER_H_
#define SERVER_MATH_RESAMPLER_H_

#include <server/common/TimeStamp.h>
#include <server/common/Span.h>
#include <server/common/TimedValue.h>

namespace sail {
namespace SampleUtils {

struct Endpoint {
  sail::TimeStamp pos;
  bool rising;

  bool operator<(const Endpoint &other) const {
    return pos < other.pos;
  }
};

Array<Endpoint> listEndpoints(const Array<TimeStamp> &samples,
    Duration<double> period);
Array<TimeStamp> makeNewSamplesFromEndpoints(const Array<Endpoint> &eps,
    Duration<double> period);
Array<Span<TimeStamp>> makeContinuousSpansWithMargin(
    const Array<TimeStamp> &timeSamples,
    Duration<double> margin);
Array<TimeStamp> resample(const Array<TimeStamp> &irregularSamples,
    Duration<double> newSamplingPeriod);
bool isCovered(TimeStamp t, const Array<Span<TimeStamp>> &spans);
Array<Span<TimeStamp>> makeGoodSpans(
    const Array<TimedValue<bool>>& good,
    Duration<double> goodMargin, Duration<double> badMaring);


}
}



#endif /* SERVER_MATH_RESAMPLER_H_ */
