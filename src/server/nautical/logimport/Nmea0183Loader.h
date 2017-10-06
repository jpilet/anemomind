/*
 * Nmea0183Loader.h
 *
 *  Created on: May 27, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_LOGIMPORT_NMEA0183LOADER_H_
#define SERVER_NAUTICAL_LOGIMPORT_NMEA0183LOADER_H_

#include <iosfwd>
#include <server/nautical/logimport/LogAccumulator.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <server/nautical/logimport/SourceGroup.h>
#include <string>

namespace sail {
namespace Nmea0183Loader {


TimeStamp updateLastTime(const TimeStamp &current, const TimeStamp &candidate);

template <typename T>
TimeStamp timestampOrUndefined(T x) {
  return TimeStamp();
}

class LogLoaderNmea0183Parser : public NmeaParser {
public:
  LogLoaderNmea0183Parser(LogAccumulator *dst,
      const std::string &s);
  void setProtobufTime(const TimeStamp &time);
protected:
  void onXDRRudder(const char *senderAndSentence,
                           bool valid,
                           sail::Angle<double> angle,
                           const char *whichRudder);
private:
  std::string _sourceName;
  LogAccumulator *_dst;
  TimeStamp _protobufTime;
};

class Nmea0183LogLoaderAdaptor {
 public:
  Nmea0183LogLoaderAdaptor(
      bool adjustTimeFromNmea0183,
      NmeaParser *parser, LogAccumulator *dst,
      const std::string &srcName) :
    _adjustTimeFromNmea0183(adjustTimeFromNmea0183),
    _parser(parser), _sourceName(srcName), _dst(dst) {}

  template <DataCode Code>
  void add(const std::string &sourceName, const typename TypeForCode<Code>::type &value) {
    typedef typename TypeForCode<Code>::type T;
    typedef typename TimedSampleCollection<T>::TimedVector TimedVector;
    std::map<std::string, TimedVector> *m = getChannels<Code>(_dst);
    auto dst = allocateSourceIfNeeded<T>(_sourceName, m);

    // If we are trying to compute a time correction offset,
    // we probably *don't* want to do this...
    if (_adjustTimeFromNmea0183) {
      setTime(timestampOrUndefined(value));
    }

    if (_lastTime.defined() && isFinite(value)) {
      dst->push_back(TimedValue<T>(_lastTime, value));
    }
  }

  void setTimeOfDay(int hour, int minute, int second) {

  }

  void setTime(const TimeStamp& time) {
    _lastTime = updateLastTime(_lastTime, time);
  }

  const std::string &sourceName() const {return _sourceName;}
 private:
  bool _adjustTimeFromNmea0183 = false;
  TimeStamp _lastTime;

  std::string _sourceName;
  NmeaParser *_parser;
  LogAccumulator *_dst;
};

std::string getDefaultSourceName();

void streamToNmeaParser(const std::string &src,
    NmeaParser *dstParser,
    Nmea0183LogLoaderAdaptor *adaptor);
void streamToNmeaParser(std::istream *src, NmeaParser *dstParser,
    Nmea0183LogLoaderAdaptor *adaptor);


void loadNmea0183Stream(
    std::istream *stream,
    LogAccumulator *dst,
    const std::string &srcName);

void loadNmea0183File(const std::string &filename, LogAccumulator *dst);


}
}

#endif /* SERVER_NAUTICAL_LOGIMPORT_NMEA0183LOADER_H_ */
