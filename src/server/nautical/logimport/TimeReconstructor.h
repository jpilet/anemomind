/*
 * TimeReconstructor.h
 *
 *  Created on: Jul 14, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_LOGIMPORT_TIMERECONSTRUCTOR_H_
#define SERVER_NAUTICAL_LOGIMPORT_TIMERECONSTRUCTOR_H_

#include <server/common/TimeStamp.h>
#include <functional>
#include <server/common/Span.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

struct LineFitSettings {
    int iterations = 4;
    double threshold = 1.0e-9;
    double reg = 1.0e-12;
  };

struct TimeReconstructorSettings {
  LineFitSettings lineFitSettings;

  Span<Duration<double> > validSamplingPeriods = Span<Duration<double> >(
      Duration<double>::seconds(0.001),
      Duration<double>::seconds(60));

  Span<TimeStamp> validTimes = Span<TimeStamp>(
      TimeStamp::UTC(2007, 1, 1, 1, 1, 1),
      TimeStamp::UTC(2094, 1, 1, 1, 1, 1)
      );

  Duration<double> maxGap = Duration<double>::minutes(5.0);

  double maxRelativeBadCount = 0.2;
};

template <typename T>
struct IndexedValue {
  int index;
  T value;
  bool operator<(const IndexedValue<T> &other) const {
    return value < other.value;
  }
};

typedef IndexedValue<TimeStamp> IndexedTime;

std::function<TimeStamp(double)> reconstructIndexToTime(
    const Array<IndexedTime> &indexedTimes,
    const TimeReconstructorSettings &settings = TimeReconstructorSettings());

std::function<TimeStamp(double)> reconstructIndexToTimeOrConstant(
    const Array<IndexedTime> &indexedTimes,
    const TimeReconstructorSettings &settings = TimeReconstructorSettings());

Array<IndexedTime> reconstructTime(
    const Array<IndexedTime> &times,
    const TimeReconstructorSettings &settings = TimeReconstructorSettings());

bool regularizeTimesInPlace(std::vector<TimeStamp> *times,
    const TimeReconstructorSettings &settings = TimeReconstructorSettings());


}

#endif /* SERVER_NAUTICAL_LOGIMPORT_TIMERECONSTRUCTOR_H_ */
