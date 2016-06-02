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

void streamToNmeaParser(const std::string &src,
    NmeaParser *dstParser,
    Nmea0183LogLoaderAdaptor *adaptor) {
  for (auto c: src) {
    Nmea0183ProcessByte(adaptor->sourceName(), c, dstParser, adaptor);
  }
}

void streamToNmeaParser(std::istream *src, NmeaParser *dstParser,
    Nmea0183LogLoaderAdaptor *adaptor) {
  while (src->good()) {
    Nmea0183ProcessByte(adaptor->sourceName(), src->get(), dstParser, adaptor);
  }
}

void loadNmea0183Stream(std::istream *stream, LogAccumulator *dst,
    const std::string &srcName) {
  LogLoaderNmea0183Parser parser(dst, srcName);
  Nmea0183LogLoaderAdaptor adaptor(&parser, dst, srcName);
  streamToNmeaParser(stream, &parser, &adaptor);
}

void loadNmea0183File(const std::string &filename, LogAccumulator *dst) {
  std::ifstream file(filename);
  loadNmea0183Stream(&file, dst, defaultNmea0183SourceName);
}


}
}
