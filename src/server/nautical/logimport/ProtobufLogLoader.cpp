/*
 * ProtobufLogLoader.cpp
 *
 *  Created on: May 27, 2016
 *      Author: jonas
 */

#include <server/nautical/logimport/ProtobufLogLoader.h>
#include <server/nautical/logimport/LogAccumulator.h>
#include <server/nautical/logimport/Nmea0183Loader.h>
#include <server/common/logging.h>
#include <server/common/LineKM.h>
#include <server/math/Majorize.h>
#include <server/math/QuadForm.h>
#include <vector>
#include <server/common/ArrayIO.h>
#include <server/common/Span.h>

namespace sail {
namespace ProtobufLogLoader {


namespace {

  const char badTimeMsg[] =
      "The times in this channel were so crappy that we will just drop them!";

  struct LineFitSettings {
    int iterations = 4;
    double threshold = 1.0e-9;
    double reg = 1.0e-12;
  };


  Optional<LineKM> iterateRobustFit(
      const Arrayd &values,
      const LineKM &line,
      const LineFitSettings &settings) {
    sail::LineFitQF qf = sail::LineFitQF::makeReg(settings.reg);
    int n = values.size();
    for (int i = 0; i < n; i++) {
      auto weight =
          MajQuad::majorizeAbs(values[i] - line(i), settings.threshold).a;
      qf += weight*sail::LineFitQF::fitLine(i, values[i]);
    }
    double km[2] = {0, 0};
    if (!qf.minimize2x1(km)) {
      LOG(WARNING) << "Failed to minimize quadratic form from "
          << values.size() << " values starting from " << line;
      if (n < 30) {
        std::stringstream ss;
        for (auto v: values) {
          ss << v << " ";
        }
        LOG(WARNING) << "  The values were " << ss.str();
      }
      return Optional<LineKM>();
    }
    return LineKM(km[0], km[1]);
  }

  Optional<LineKM> fitStraightLineRobustly(
      const LineKM &init,
      const Arrayd &values,
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

  double initializeSlope(const Arrayd &values) {
    CHECK(2 <= values.size());
    int preferredStep = 3;
    int maxStep = values.size() - 1;
    int step = std::min(preferredStep, maxStep);
    CHECK(1 <= step);
    int n = values.size() - step;

    std::vector<double> steps;
    steps.reserve(n);
    for (int i = 0; i < n; i++) {
      auto slopeValue = (values[i+step] - values[i])/step;
      steps.push_back(slopeValue);
    }
    assert(steps.size() == n);

    auto middle = steps.begin() + n/2;
    std::nth_element(steps.begin(), middle, steps.end());
    double slope = *middle;
    return slope;
  }

  struct IndexedTimeStamp {
    int index;
    TimeStamp time;

    bool operator<(const IndexedTimeStamp &other) const {
      return time < other.time;
    }
  };

  IndexedTimeStamp getMedianTimeStamp(const std::vector<TimeStamp> &times) {
    int n = times.size();
    std::vector<IndexedTimeStamp> times2;
    times2.reserve(n);
    for (int i = 0; i < n; i++) {
      times2.push_back(IndexedTimeStamp{i, times[i]});
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
}

/*
 * We would expect that in most cases, an index 'x'
 * in a vector of timestamps 'y' maps to 'y' as
 *
 *    y = k*x + m
 *
 * This function robustly finds 'k' and 'm' and adjusts all times
 * 'y' in the vector that fit very badly with this line.
 */
bool regularizeTimesInPlace_(std::vector<TimeStamp> *times) {

  Span<Duration<double> > validSamplingPeriods(
      Duration<double>::seconds(0.001),
      Duration<double>::seconds(60));

  int n = times->size();
  if (2 <= n) {
    auto median = getMedianTimeStamp(*times);
    Arrayd secondsToOffset(n);
    for (int i = 0; i < n; i++) {
      secondsToOffset[i] = ((*times)[i] - median.time).seconds();
    }

    double initSlope = initializeSlope(secondsToOffset);

    auto initialLineFit = LineKM(initSlope, -initSlope*median.index);
    auto index2timeSeconds0 = fitStraightLineRobustly(
        initialLineFit,
        secondsToOffset, LineFitSettings());

    if (!index2timeSeconds0.defined()) {
      return false;
    }
    auto index2timeSeconds = index2timeSeconds0.get();

    auto samplingPeriod = Duration<double>::seconds(index2timeSeconds.getK());
    if (!validSamplingPeriods.contains(samplingPeriod)) {
      LOG(ERROR) << "Fitted sampling period "
          << samplingPeriod.seconds() << " seconds out of bounds";
      return false;
    }

    Duration<double> maxGap = Duration<double>::minutes(5.0);
    int badCounter = 0;

    for (int i = 0; i < n; i++) {
      auto &time = (*times)[i];
      auto predicted = Duration<double>::seconds(index2timeSeconds(i)) + median.time;
      auto gap = fabs(time - predicted);
      if (maxGap < gap) {
        badCounter++;
        time = predicted;
      }
    }
    if (0 < badCounter) {
      LOG(WARNING) << "When loading some log file, " << badCounter <<
          " of " << n <<
          " times for some channel were really bad and had to be adjusted.";
      LOG(INFO) << "   Initial line fit: " << initialLineFit;
      LOG(INFO) << "   Optimized line fit: " << index2timeSeconds;
      LOG(INFO) << "   Offset time stamp: " << median.time;
      LOG(INFO) << "   The adjusted vector spans times from "
          << times->front() << " to " << times->back();
    }
    if (0.2*n < badCounter) {
      LOG(ERROR) << "In fact, the proportion of bad samples is unacceptable.";
      return false;
    }
  }
  return true;
}

bool regularizeTimesInPlace(std::vector<TimeStamp> *times) {
  auto result = regularizeTimesInPlace_(times);
  return result && allTimesAreValid(*times);
}

namespace {
  struct OffsetWithFitnessError {
    OffsetWithFitnessError() {
      defaultConstructed = true;
      offset = Duration<double>::seconds(0.0);
      averageErrorFromMedian = std::numeric_limits<double>::infinity();
      priority = (-std::numeric_limits<int>::max());
    }

    OffsetWithFitnessError(Duration<double> dur, double e, int p) :
      offset(dur), averageErrorFromMedian(e), priority(p),
      defaultConstructed(false) {}

    bool defaultConstructed;
    int priority;
    Duration<double> offset;
    double averageErrorFromMedian;

    std::pair<int, double> makePairToMinimize() const {
      // First, try to minimize the **negated** priority (which is the same as maximizing the priority)
      // Second, try to minimize the error
      return std::make_pair(-priority, averageErrorFromMedian);
    }

    bool operator<(const OffsetWithFitnessError &e) const {
      return makePairToMinimize() < e.makePairToMinimize();
    }
  };
}

/**
 * Log file loading coded in our format (using protobuf)
 */
template <typename T>
void addToVector(const ValueSet &src, const OffsetWithFitnessError &offset,
    std::deque<TimedValue<T> > *dst) {
  std::vector<TimeStamp> timeVector;
  std::vector<T> dataVector;
  ValueSetToTypedVector<TimeStamp>::extract(src, &timeVector);
  ValueSetToTypedVector<T>::extract(src, &dataVector);

  CHECK(offset.defaultConstructed);
  if (offset.defaultConstructed) {
    if (!regularizeTimesInPlace(&timeVector)) {
      LOG(ERROR) << badTimeMsg;
      return;
    }
  }

  auto n = dataVector.size();
  if (n == dataVector.size()) {
    for (size_t i = 0; i < n; i++) {
      dst->push_back(TimedValue<T>(timeVector[i] + offset.offset, dataVector[i]));
    }
  } else {
    LOG(WARNING) << "Incompatible time and data vector sizes. Ignore this data.";
  }
}

void analyzeTimes(const std::vector<TimeStamp> times) {
  int n = times.size() - 1;
  Duration<double> maxStep = Duration<double>::seconds(0.0);
  for (int i = 0; i < n; i++) {
    maxStep = std::max(maxStep, times[i] - times[i+1]);
  }

  if (maxStep > Duration<double>::hours(2.0)) {
    std::cout << "Max step backwards in time: " << maxStep.seconds() << std::endl;
  }
}

void loadTextData(const ValueSet &stream, LogAccumulator *dst,
    const OffsetWithFitnessError &offset) {
  vector<TimeStamp> times;
  Logger::unpackTime(stream, &times);

  CHECK(offset.defaultConstructed);
  if (offset.defaultConstructed) {
    if (!regularizeTimesInPlace(&times)) {
      LOG(ERROR) << badTimeMsg;
    }
  }

  auto n = stream.text_size();
  if (n == 0) {
    return;
  } else if (n > times.size()) {
    LOG(WARNING) << "Omitting text data, because incompatible sizes "
        << n << " and " << times.size();
  } else {
    std::string originalSourceName = stream.source();
    std::string dstSourceName = originalSourceName + " reparsed";
    Nmea0183Loader::LogLoaderNmea0183Parser parser(dst, dstSourceName);
    Nmea0183Loader::Nmea0183LogLoaderAdaptor adaptor(&parser, dst, dstSourceName);
    for (int i = 0; i < n; i++) {
      auto rawTime = times[i];
      auto t = times[i] + offset.offset;
      parser.setProtobufTime(t);
      adaptor.setTime(t);
      streamToNmeaParser(stream.text(i), &parser, &adaptor);
    }
  }
}

void loadValueSet(const ValueSet &stream, LogAccumulator *dst,
    const OffsetWithFitnessError &offset) {
#define ADD_VALUES_TO_VECTOR(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  if (stream.shortname() == SHORTNAME) {addToVector<TYPE>(stream, offset, &(dst->_##HANDLE##sources[stream.source()]));}
      FOREACH_CHANNEL(ADD_VALUES_TO_VECTOR)
#undef  ADD_VALUES_TO_VECTOR
  loadTextData(stream, dst, offset);
}

namespace {

  OffsetWithFitnessError computeTimeOffset(const ValueSet &stream) {
    std::vector<Duration<double> > diffs;
    std::vector<TimeStamp> extTimes;
    Logger::unpack(stream.exttimes(), &extTimes);
    if (!extTimes.empty()) {
      std::vector<TimeStamp> times;
      Logger::unpackTime(stream, &times);
      if (times.size() == extTimes.size()) {
        int n = times.size();
        for (int j = 0; j < n; j++) {
          diffs.push_back(extTimes[j] - times[j]);
        }
      } else {
        LOG(WARNING) << "Inconsistent size of times and exttimes for stream";
      }
    }
    auto n = diffs.size();
    if (n > 30) { // Sufficiently many?
      auto at = diffs.begin() + n/2;
      std::nth_element(diffs.begin(), at, diffs.end());
      auto median = *at;
      double totalError = 0.0;
      for (auto x: diffs) {
        totalError += std::abs((x - median).seconds());
      }
      return OffsetWithFitnessError(median, totalError/n, stream.priority());
    }
    return OffsetWithFitnessError();
  }

  OffsetWithFitnessError computeTimeOffset(const LogFile &data) {
    OffsetWithFitnessError offset;
    for (int i = 0; i < data.stream_size(); i++) {
      auto c = computeTimeOffset(data.stream(i));
      offset = std::min(offset, c);
    }
    return offset;
  }
}


void load(const LogFile &data, LogAccumulator *dst) {
  // TODO: Define a set of standard priorities in a file somewhere
  auto rawStreamPriority = -16;

  auto timeOffset = computeTimeOffset(data);

  for (int i = 0; i < data.stream_size(); i++) {
    const auto &stream = data.stream(i);
    dst->_sourcePriority[stream.source()] = stream.priority();
    loadValueSet(stream, dst, timeOffset);
  }

  for (int i = 0; i < data.text_size(); i++) {
    const auto &stream = data.text(i);
    dst->_sourcePriority[stream.source()] = rawStreamPriority;
    loadValueSet(stream, dst, timeOffset);
  }
}

bool load(const std::string &filename, LogAccumulator *dst) {
  LogFile file;
  if (Logger::read(filename, &file)) {
    load(file, dst);
    return true;
  }
  return false;
}

}
} /* namespace sail */
