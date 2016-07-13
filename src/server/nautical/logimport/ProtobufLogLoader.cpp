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

namespace sail {
namespace ProtobufLogLoader {


namespace {

  struct LineFitSettings {
    int iterations = 4;
    double threshold = 1.0e-9;
    double reg = 1.0e-12;
  };


  LineKM iterateRobustFit(
      const Arrayd &values,
      const LineKM &line,
      const LineFitSettings &settings) {
    sail::LineFitQF qf = sail::LineFitQF::makeReg(settings.reg);
    int n = values.size();
    std::cout << "  --- Line " << line << std::endl;
    for (int i = 0; i < n; i++) {
      auto weight =
          MajQuad::majorizeAbs(values[i] - line(i), settings.threshold).a;
      qf += weight*sail::LineFitQF::fitLine(i, values[i]);
    }
    double km[2] = {0, 0};
    qf.minimize2x1(km);
    return LineKM(km[0], km[1]);
  }

  LineKM fitStraightLineRobustly(
      const LineKM &init,
      const Arrayd &values,
      const LineFitSettings &settings) {
    LineKM line = init;
    for (int i = 0; i < settings.iterations; i++) {
      line = iterateRobustFit(values, line, settings);
    }
    return line;
  }

  double initializeSlope(const Arrayd &values) {
    int preferredStep = 1;
    int maxStep = values.size() - 1;
    int step = std::min(preferredStep, maxStep);
    int n = values.size() - step;

    std::vector<double> steps;
    steps.reserve(n);
    for (int i = 0; i < n; i++) {
      steps.push_back((values[i+step] - values[i])/step);
    }
    assert(steps.size() == n);
    auto middle = steps.begin() + steps.size()/2;
    std::nth_element(steps.begin(), steps.end(), middle);
    return *middle;
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
    std::nth_element(times2.begin(), times2.end(), middle);
    return *middle;
  }
}

void regularizeTimesInPlace(std::vector<TimeStamp> *times) {
  int n = times->size();
  if (0 < n) {
    auto median = getMedianTimeStamp(*times);
    Arrayd secondsToOffset(n);
    for (int i = 0; i < n; i++) {
      secondsToOffset[i] = ((*times)[i] - median.time).seconds();
    }

    double slope = initializeSlope(secondsToOffset);

    auto index2timeSeconds = fitStraightLineRobustly(
        LineKM(slope, -slope*median.index),
        secondsToOffset, LineFitSettings());
  }
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

  if (offset.defaultConstructed) {
    regularizeTimesInPlace(&timeVector);
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

  if (offset.defaultConstructed) {
    regularizeTimesInPlace(&times);
  }
  analyzeTimes(times);

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
