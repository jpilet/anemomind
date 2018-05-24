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
#include <fstream>

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

typedef std::function<void(TimeStamp)> TimedOperation;

class Nmea0183TimeFuser : boost::noncopyable {
public:
  Nmea0183TimeFuser() {}

  void setTime(TimeStamp t);
  void setTimeSinceMidnight(Duration<double> d);
  void bufferOperation(TimedOperation op);

  ~Nmea0183TimeFuser() {flush();}
private:
  void flush();
/*
 * Let T=1 iff _lastTime defined, T=0 iff _lastTime undefined,
 * Let L=1 iff _lastTimeSinceMidnight defined, L=0 it is undefined
 *
 * Four possiblities when buffering an operation
 *
 *   - T=0, L=0 (possible just after object instantiation for instance)
 *     => We cannot assign any time to the operation, so drop it.
 *
 *   - T=1, L=0 (possible in case only setTime has been called so far.)
 *     => Perform the operation *now* using the value of _lastTime
 *
 *   - T=0, L=1 (possible in case only setTimeSinceMidnight has been called so far)
 *     => Put the operation in the buffer with the value of _lastTimeSinceMidnight
 *
 *   - T=1, L=1
 *     => (Same as for T=0, L=1)
 *
 *  Note that buffering happens iff L=1.
 */

  // Last full time provided
  TimeStamp _lastTime;
  Optional<Duration<double>> _lastTimeSinceMidnight;
  std::vector<std::pair<Duration<double>, TimedOperation>> _delayedOps;
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
    if (isFinite(value)) {
      // This puts the operation in a buffer, and
      // it will be performed once we have a good time estimate.
      _time.bufferOperation([dst, value](TimeStamp timeEst) {
        dst->push_back(TimedValue<T>(timeEst, value));
      });
    }
  }

  void setTimeOfDay(int hour, int minute, int second) {
    _time.setTimeSinceMidnight(
          double(hour)*1.0_hours
        + double(minute)*1.0_minutes
        + double(second)*1.0_s);
  }

  void setTime(const TimeStamp& time) {
    _time.setTime(time);
  }

  const std::string &sourceName() const {return _sourceName;}
 private:
  bool _adjustTimeFromNmea0183 = false;
  Nmea0183TimeFuser _time;

  std::string _sourceName;
  NmeaParser *_parser;
  LogAccumulator *_dst;
};

std::string getDefaultSourceName();

void streamToNmeaParser(TimeStamp endTime,
                        const std::string &src,
                        Duration<> interval,
                        LogLoaderNmea0183Parser *dstParser,
                        Nmea0183LogLoaderAdaptor *adaptor);

void streamToNmeaParser(std::istream *src, NmeaParser *dstParser,
    Nmea0183LogLoaderAdaptor *adaptor);


bool loadNmea0183Stream(
    std::istream *stream,
    LogAccumulator *dst,
    const std::string &srcName);

bool loadNmea0183File(const std::string &filename, LogAccumulator *dst);


}
}

#endif /* SERVER_NAUTICAL_LOGIMPORT_NMEA0183LOADER_H_ */
