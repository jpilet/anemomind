/*
 * Nmea0183Loader.cpp
 *
 *  Created on: May 27, 2016
 *      Author: jonas
 */

#include "Nmea0183Loader.h"
#include <server/common/TimeStamp.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <device/anemobox/Nmea0183Adaptor.h>
#include <server/nautical/logimport/SourceGroup.h>
#include <fstream>
#include <server/common/IntervalUtils.h>

namespace sail {
namespace Nmea0183Loader {

namespace {
  Arrayi makeIndexToIntervalMap(int n, Array<IndexedTime> times) {
    Arrayi dst = Arrayi::fill(n, -1);
    int from = 0;
    int to = 0;
    for (int i = 0; i < times.size() - 1; i++) {
      from = times[i].index;
      to = times[i+1].index;
      for (int j = from; j < to; j++) {
        dst[j] = i;
      }
    }
    for (int i = to; i < n; i++) {
      dst[i] = times.size();
    }
    return dst;
  }
}

std::function<TimeStamp(int)> makeNmeaIndexToTimeFunction(
    int n, const Array<IndexedTime> &times) {
  for (auto t: times) {
    CHECK(t.value.defined());
  }

  // Precompute a table that maps a sample index
  // to the index of an interval between two time stamps.
  Arrayi idx2ivl = makeIndexToIntervalMap(n, times);

  return [=](int x) {
    if (x < 0 || n <= x) {
      return TimeStamp();
    }
    int intervalIndex = idx2ivl[x];

    // First, any measurement must be surrounded by a time stamp on either side
    if (intervalIndex == -1) {
      return TimeStamp();
    } else if (intervalIndex == times.size()) {
      return TimeStamp();
    }

    auto left = times[intervalIndex];
    auto right = times[intervalIndex+1];

    // Second, if the measurement is surrounded, the time from one
    // time stamp to the next should be positive but not longer than, say,
    // 1 minute or so..
    if (right.value < left.value) {
      return TimeStamp();
    } else if (Duration<double>::minutes(1.0) < right.value - left.value) {
      return TimeStamp();
    }

    // The times seem OK, so interpolate based on the index.
    auto lambda = computeLambda<double>(left.index, right.index, x);
    auto time = left.value + lambda*(right.value - left.value);

    // But lastly, also check that the timestamp is no older than our first
    // recordings.
    return time < TimeStamp::UTC(2008, 1, 1, 1, 1, 1)? TimeStamp() : time;
  };
}

LogLoaderNmea0183Parser::LogLoaderNmea0183Parser(
    LogAccumulator *dst,
  const std::string &s,
  bool ignoreWrongChecksum) : _dst(dst), _sourceName(s) {
  setIgnoreWrongChecksum(ignoreWrongChecksum);
}

void LogLoaderNmea0183Parser::setProtobufTime(const TimeStamp &time) {
  _protobufTime = time;
}

void LogLoaderNmea0183Parser::onXDRRudder(const char *senderAndSentence,
                         bool valid,
                         sail::Angle<double> angle,
                         const char *whichRudder) {
  auto t = latest(timestamp(), _protobufTime);
  if (valid && t.defined()) {
    (*(_dst->getRUDDER_ANGLEsources()))[_sourceName].push_back(
        TimedValue<Angle<double> >(t, angle));
  }
}


std::string defaultNmea0183SourceName = "NMEA0183";

std::string getDefaultSourceName() {
  return defaultNmea0183SourceName;
}

void streamToNmeaParser(const std::string &nmeaMessage,
    NmeaParser *dstParser,
    Nmea0183LogLoaderAdaptor *adaptor) {
  for (auto c: nmeaMessage) {
    Nmea0183ProcessByte(c, dstParser, adaptor);
  }
}

void streamToNmeaParser(std::istream *src, NmeaParser *dstParser,
    Nmea0183LogLoaderAdaptor *adaptor) {
  while (src->good()) {
    Nmea0183ProcessByte(src->get(), dstParser, adaptor);
  }
}

void loadNmea0183Stream(std::istream *stream, LogAccumulator *dst,
    const std::string &srcName,
    const LogLoaderSettings &settings) {
  LogLoaderNmea0183Parser parser(dst, srcName, settings.ignoreNmea0183Checksum);
  Nmea0183LogLoaderAdaptor adaptor(&parser);
  streamToNmeaParser(stream, &parser, &adaptor);
  adaptor.outputTo(srcName, dst);
}

void loadNmea0183File(const std::string &filename, LogAccumulator *dst,
    const LogLoaderSettings &settings) {
  std::ifstream file(filename);
  loadNmea0183Stream(&file, dst, defaultNmea0183SourceName, settings);
}


}
}
