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
#include <vector>
#include <server/nautical/BoatSpecificHacks.h>

namespace sail {
namespace ProtobufLogLoader {

namespace {

// For all sort of strange reasons, we receive weird dates.
// We'll assume that everything before the 1st GPS Week Number Rollover
// in 1999 is invalid.
// We'll assume that anything between 1999 and 2014 is 1024 weeks too old.
// (see FixGpsBug below)
const TimeStamp kMinValidTime = TimeStamp::UTC(1999, 8, 21, 0, 0, 0);
// We refuse recording in the future.
const TimeStamp kMaxValidTime = TimeStamp::now() + Duration<>::hours(24);

const TimeStamp kGpsBugStart = TimeStamp::fromMilliSecondsSince1970(0)
      + Duration<double>::weeks(1024);
const TimeStamp kGpsBugEnd = TimeStamp::UTC(2014, 1, 1, 0, 0, 0.0);

}  // namespace

/**
 * Log file loading coded in our format (using protobuf)
 */
template <typename T>
void addToVector(const ValueSet &src, Duration<double> offset,
    std::deque<TimedValue<T> > *dst) {
  std::vector<TimeStamp> timeVector;
  std::vector<T> dataVector;
  ValueSetToTypedVector<TimeStamp>::extract(src, &timeVector);
  ValueSetToTypedVector<T>::extract(src, &dataVector);
  auto n = dataVector.size();
  if (n == dataVector.size()) {
    for (size_t i = 0; i < n; i++) {
      dst->push_back(TimedValue<T>(timeVector[i] + offset, dataVector[i]));
    }
  } else {
    LOG(WARNING) << "Incompatible time and data vector sizes. Ignore this data.";
  }
}

void loadTextData(const ValueSet &stream, LogAccumulator *dst,
    Duration<double> offset) {
  vector<TimeStamp> times;
  Logger::unpackTime(stream, &times);

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
    Nmea0183Loader::Nmea0183LogLoaderAdaptor adaptor(
        false, // <-- All the text log data are string chunks that are tagged
               // with times that we correct with a provided offset, so
               // we rely on those values, rather than the time values that we
               // get from the NMEA0183 data.
        &parser, dst, dstSourceName);

    int byteCount = 0;
    for (int i = 0; i < n; i++) {
      byteCount += stream.text(i).size();
    }
    Duration<> interval = byteCount > 0 ? 
      (times[n - 1] - times[0]).scaled(1.0 / double(byteCount))
      : Duration<>::seconds(1/4800.0);

    for (int i = 0; i < n; i++) {
      auto t = times[i] + offset;
      streamToNmeaParser(t, stream.text(i), interval, &parser, &adaptor);
    }
  }
}

void loadValueSet(const ValueSet &stream, LogAccumulator *dst,
    Duration<double> offset) {
#define ADD_VALUES_TO_VECTOR(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  if (stream.shortname() == SHORTNAME) {addToVector<TYPE>(stream, offset, &(dst->_##HANDLE##sources[stream.source()]));}
      FOREACH_CHANNEL(ADD_VALUES_TO_VECTOR)
#undef  ADD_VALUES_TO_VECTOR
  loadTextData(stream, dst, offset);
}

namespace {

  TimeStamp FixGpsBug(TimeStamp maybeBugged) {
    TimeStamp result = maybeBugged;
    if (maybeBugged > kGpsBugStart && maybeBugged < kGpsBugEnd) {
       result += Duration<double>::weeks(1024);
    }
    return result;
  }

  struct OffsetWithFitnessError {
    static const int initPriority = (-std::numeric_limits<int>::max());
    static constexpr double initError = std::numeric_limits<double>::infinity();

    OffsetWithFitnessError() {
      offset = Duration<double>::seconds(0.0);
      averageErrorFromMedian = initError;
      priority = initPriority;
    }

    OffsetWithFitnessError(Duration<double> dur, double e, int p) :
      offset(dur), averageErrorFromMedian(e), priority(p) {
    }

    bool defined() const {
      return averageErrorFromMedian < initError;
    }

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

  std::ostream& operator<<(
      std::ostream& s, const OffsetWithFitnessError& x) {
    s << "\n offset:   " << x.offset.str()
      << "\n priority: " << x.priority
      << "\n avg err:  " << x.averageErrorFromMedian;
    return s;
  }

  OffsetWithFitnessError computeTimeOffset(const ValueSet &stream) {
    std::vector<Duration<double> > diffs;
    std::vector<TimeStamp> extTimes;
    Logger::unpack(stream.exttimes(), &extTimes);

    std::vector<TimeStamp> times;

    if (extTimes.empty()) {
      // extTimes is empty, but maybe we do have time info in GLL sentences
      // in: 'text[NMEA0183 input:0]'
      LogAccumulator acc;
      loadTextData(stream, &acc, Duration<double>::seconds(0));
      std::map<std::string, TimedSampleCollection<TimeStamp>::TimedVector>* map =
	acc.getDATE_TIMEsources();
      if (map->size() > 0) {
	const TimedSampleCollection<TimeStamp>::TimedVector& values(
	    map->begin()->second);

	for (const TimedValue<TimeStamp>& it : values) {
	  extTimes.push_back(it.value);
	  times.push_back(it.time);
	}
      }
    } else {
      Logger::unpackTime(stream, &times);
    }

    if (times.size() == extTimes.size()) {
      int n = times.size();
      for (int j = 0; j < n; j++) {
	if (extTimes[j].defined() && times[j].defined()
            && extTimes[j] > kMinValidTime
            && extTimes[j] < kMaxValidTime) {
	  diffs.push_back(FixGpsBug(extTimes[j]) - times[j]);
	}
      }
    } else {
      LOG(WARNING) << "Inconsistent size of times and exttimes for stream";
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

  void forAllValueSets(
      const LogFile& data,
      const std::function<void(ValueSet)>& f) {
    for (int i = 0; i < data.stream_size(); i++) {
      f(data.stream(i));
    }
    for (int i = 0; i < data.text_size(); i++) {
      f(data.text(i));
    }
  }

  Duration<double> computeTimeOffset(const LogFile &data) {
    OffsetWithFitnessError offset;
    forAllValueSets(data, [&](const ValueSet& stream) {
      auto c = computeTimeOffset(stream);
      offset = std::min(offset, c);
    });
    return offset.offset;
  }
}


void load(const LogFile &data, LogAccumulator *dst) {

  hack::bootCount = data.bootcount() - 101;

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
