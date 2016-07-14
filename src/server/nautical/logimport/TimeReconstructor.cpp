/*
 * TimeReconstructor.cpp
 *
 *  Created on: Jul 14, 2016
 *      Author: jonas
 */

#include <server/nautical/logimport/TimeReconstructor.h>
#include <server/common/logging.h>
#include <server/common/LineKM.h>
#include <server/common/Optional.h>
#include <server/common/LineKM.h>
#include <server/math/Majorize.h>
#include <server/math/QuadForm.h>
#include <vector>
#include <server/common/ArrayIO.h>
#include <server/common/Span.h>
#include <server/common/ArrayBuilder.h>

namespace sail {

  Optional<LineKM> iterateRobustFit(
      const Array<IndexedValue<double> > &values,
      const LineKM &line,
      const LineFitSettings &settings) {
    sail::LineFitQF qf = sail::LineFitQF::makeReg(settings.reg);
    int n = values.size();
    for (auto v: values) {
      auto weight =
          MajQuad::majorizeAbs(v.value - line(v.index), settings.threshold).a;
      qf += weight*sail::LineFitQF::fitLine(v.index, v.value);
    }
    double km[2] = {0, 0};
    if (!qf.minimize2x1(km)) {
      LOG(WARNING) << "Failed to minimize quadratic form from "
          << values.size() << " values starting from " << line;
      if (n < 30) {
        std::stringstream ss;
        for (auto v: values) {
          ss << "(" << v.index << ", " << v.value << ") ";
        }
        LOG(WARNING) << "  The values were " << ss.str();
      }
      return Optional<LineKM>();
    }
    return LineKM(km[0], km[1]);
  }

  Optional<LineKM> fitStraightLineRobustly(
      const LineKM &init,
      const Array<IndexedValue<double> > &values,
      const LineFitSettings &settings) {
    LineKM line = init;
    for (int i = 0; i < settings.iterations; i++) {
      auto l = iterateRobustFit(values, line, settings);
      if (l.defined()) {
        line = l.get();
      } else {
        return Optional<LineKM>();
      }
    }
    return line;
  }

  Optional<double> initializeSlope(const Array<IndexedValue<double> > &values) {
    CHECK(2 <= values.size());
    int preferredStep = 3;
    int maxStep = values.size() - 1;
    int step = std::min(preferredStep, maxStep);
    CHECK(1 <= step);
    int n = values.size() - step;

    std::vector<double> steps;
    steps.reserve(n);
    for (int i = 0; i < n; i++) {
      auto a = values[i];
      auto b = values[i+step];
      auto denom = b.index - a.index;
      if (denom > 0) {
        steps.push_back((1.0/denom)*(b.value - a.value));
      }
    }
    if (steps.empty()) {
      return Optional<double>();
    }

    auto middle = steps.begin() + n/2;
    std::nth_element(steps.begin(), middle, steps.end());
    double slope = *middle;
    return slope;
  }

  IndexedTime getMedianTimeStamp(const Array<IndexedTime> &times) {
    int n = times.size();
    std::vector<IndexedTime> times2;
    times2.reserve(n);
    for (auto x: times) {
      times2.push_back(x);
    }
    assert(times2.size() == times.size());
    auto middle = times2.begin() + n/2;
    std::nth_element(times2.begin(), middle, times2.end());
    return *middle;
  }

  Span<TimeStamp> validTimes(TimeStamp::UTC(2014, 1, 1, 1, 0, 0),

                             // This is not that nice :-(
                             TimeStamp::UTC(2034, 1, 1, 1, 0, 0));

  bool allTimesAreValid(const std::vector<TimeStamp> &times) {
    for (auto t: times) {
      if (!validTimes.contains(t)) {
        LOG(ERROR) << "Invalid time: " << t;
        return false;
      }
    }
    return true;
  }


Array<IndexedTime> makeIndexedTime(const std::vector<TimeStamp> &times) {
  int n = times.size();
  Array<IndexedTime> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = IndexedTime{i, times[i]};
  }
  return dst;
}

Array<IndexedValue<double> > toLocalTimes(const Array<IndexedTime> &times,
    const IndexedTime &offset) {
  int n = times.size();
  ArrayBuilder<IndexedValue<double> > dst;
  for (auto t: times) {
    dst.add(IndexedValue<double>{t.index, (t.value - offset.value).seconds()});
  }
  return dst.get();
}

Span<int> getIndexSpan(const Array<IndexedTime> &src) {
  Span<int> span;
  for (auto x: src) {
    span.extend(x.index);
  }
  return span;
}

std::function<TimeStamp(double)> reconstructIndexToTime(
    const Array<IndexedTime> &indexedTimes,
    const TimeReconstructorSettings &settings) {
  if (indexedTimes.size() < 2) {
    LOG(ERROR) << "Too few samples to reconstruct time";
    return std::function<TimeStamp(double)>();
  }

  int n = indexedTimes.size();

  auto median = getMedianTimeStamp(indexedTimes);
  Array<IndexedValue<double> > local = toLocalTimes(indexedTimes, median);

  auto initSlope0 = initializeSlope(local);
  if (initSlope0.undefined()) {
    LOG(ERROR) << "Unable to compute initial slope";
    return std::function<TimeStamp(double)>();
  }
  auto initSlope = initSlope0.get();

  auto initialLineFit = LineKM(initSlope, -initSlope*median.index);
  auto index2timeSeconds0 = fitStraightLineRobustly(
      initialLineFit,
      local, settings.lineFitSettings);

  if (!index2timeSeconds0.defined()) {
    LOG(ERROR) << "Line fit failed";
    return std::function<TimeStamp(double)>();
  }
  auto index2timeSeconds = index2timeSeconds0.get();
  auto samplingPeriod = Duration<double>::seconds(index2timeSeconds.getK());

  if (!settings.validSamplingPeriods.contains(samplingPeriod)) {
    LOG(ERROR) << "Fitted sampling period "
        << samplingPeriod.seconds() << " seconds out of bounds";
    return std::function<TimeStamp(double)>();
  }

  auto f = [=](double index) {
    return median.value + Duration<double>::seconds(index2timeSeconds(index));
  };

  auto indexSpan = getIndexSpan(indexedTimes);
  if (!settings.validTimes.contains(f(indexSpan.minv()))
      || !settings.validTimes.contains(f(indexSpan.maxv()))) {
    LOG(ERROR) << "Mapped times out of range";
    return std::function<TimeStamp(double)>();
  }

  return f;
}

std::function<TimeStamp(double)> reconstructIndexToTimeOrConstant(
    const Array<IndexedTime> &indexedTimes,
    const TimeReconstructorSettings &settings) {
  if (indexedTimes.empty()) {
    LOG(ERROR) << "No samples from which to reconstruct time";
    return std::function<TimeStamp(double)>();
  } else if (indexedTimes.size() == 1) {
    return [=](double) {
      return indexedTimes[0].value;
    };
  } else {
    return reconstructIndexToTime(indexedTimes, settings);
  }
}


Array<IndexedTime> reconstructTime(const Array<IndexedTime> &times,
    const TimeReconstructorSettings &settings) {
  int n = times.size();
  ArrayBuilder<IndexedTime> dst;

  auto index2time = reconstructIndexToTime(times, settings);
  if (!index2time) {
    LOG(ERROR) << "Failed to reconstruct times because no function was fitted";
    return Array<IndexedTime>();
  }

  int badCounter = 0;

  for (auto x: times) {
    auto predicted = index2time(x.index);
    if (!settings.validTimes.contains(predicted)) {
      LOG(ERROR) << "The reconstructed time function maps to invalid times";
      return Array<IndexedTime>();
    }
    auto gap = fabs(x.value - predicted);
    if (settings.maxGap < gap) {
      badCounter++;
      dst.add(IndexedTime{x.index, predicted});
    } else {
      dst.add(x);
    }
  }

  if (0 < badCounter) {
    LOG(WARNING) << "When loading some log file, " << badCounter <<
        " of " << n <<
        " times for some channel were really bad and had to be adjusted.";
  }
  if (settings.maxRelativeBadCount*n < badCounter) {
    LOG(ERROR) << "In fact, the proportion of bad samples is unacceptable.";
    return Array<IndexedTime>();
  }

  return dst.get();
}

bool regularizeTimesInPlace(std::vector<TimeStamp> *times,
    const TimeReconstructorSettings &settings) {
  auto reconstructed = reconstructTime(makeIndexedTime(*times), settings);
  if (reconstructed.empty()) {
    return false;
  } else {
    CHECK(reconstructed.size() == times->size());
    for (auto x: reconstructed) {
      CHECK(x.index < times->size());
      (*times)[x.index] = x.value;
    }
    return true;
  }
}

}
