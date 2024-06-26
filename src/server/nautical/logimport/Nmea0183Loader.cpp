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

namespace sail {
namespace Nmea0183Loader {

TimeStamp updateLastTime(const TimeStamp &current, const TimeStamp &candidate) {
  if (!candidate.defined()) {
    return current;
  } else if (!current.defined()) {
    return candidate;
  }
  return std::max(current, candidate);
}

void Nmea0183TimeFuser::bufferOperation(TimedOperation op) {
  if (_lastTimeSinceMidnight.defined()) {
    _delayedOps.push_back({_lastTimeSinceMidnight.get(), op});
  } else if (_lastTime.defined()) {
    op(_lastTime);
  } else {
    // Drop it. No reasonable way of assigning a time to it.
  }
}

void Nmea0183TimeFuser::flush() {
  if (_lastTime.defined() && !_delayedOps.empty()) {
    CHECK(_lastTimeSinceMidnight.defined());
    for (auto op: _delayedOps) {

      // _lastTime - estimatedTimeOfOp(?) = _lastTimeSinceMidnight - op.first
      //    / where op.first is time since midnight of op /
      //  <===>
      // estimatedTimeOfOp = _lastTime - (_lastTimeSinceMidnight - op.first)
      op.second(_lastTime - (
          _lastTimeSinceMidnight.get() - op.first));
    }
    _delayedOps.clear();
  }
}

void Nmea0183TimeFuser::setTime(TimeStamp t) {
  _lastTime = updateLastTime(_lastTime, t);
  flush();
}

void Nmea0183TimeFuser::setTimeSinceMidnight(Duration<double> d) {
  _lastTimeSinceMidnight = d;
}

template <>
TimeStamp timestampOrUndefined<TimeStamp>(TimeStamp x) {
  return x;
}

LogLoaderNmea0183Parser::LogLoaderNmea0183Parser(LogAccumulator *dst,
  const std::string &s) : _dst(dst), _sourceName(s) {
  setIgnoreWrongChecksum(true);
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

void streamToNmeaParser(TimeStamp endTime,
                        const std::string &src,
                        Duration<> interval,
                        LogLoaderNmea0183Parser *dstParser,
                        Nmea0183LogLoaderAdaptor *adaptor) {
  TimeStamp t = endTime - interval.scaled(src.size());
  for (auto c: src) {
    adaptor->setTime(t);
    dstParser->setProtobufTime(t);
    t += interval;
    Nmea0183ProcessByte(adaptor->sourceName(), c, dstParser, adaptor);
  }
}

void streamToNmeaParser(std::istream *src, NmeaParser *dstParser,
    Nmea0183LogLoaderAdaptor *adaptor) {
  while (src->good()) {
    Nmea0183ProcessByte(adaptor->sourceName(), src->get(), dstParser, adaptor);
  }
}

bool loadNmea0183Stream(std::istream *stream, LogAccumulator *dst,
    const std::string &srcName) {
  LogLoaderNmea0183Parser parser(dst, srcName);
  Nmea0183LogLoaderAdaptor adaptor(
      true, // This is a separate old-style log file, so here we need to rely
            // on whatever time source there is.
      &parser, dst, srcName);
  streamToNmeaParser(stream, &parser, &adaptor);
  return parser.numSentences() > 0;
}

bool loadNmea0183File(const std::string &filename, LogAccumulator *dst) {
  std::ifstream file(filename);
  return loadNmea0183Stream(&file, dst, defaultNmea0183SourceName);
}


}
}
