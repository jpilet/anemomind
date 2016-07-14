/*
 * Nmea0183Loader.h
 *
 *  Created on: May 27, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_LOGIMPORT_Nmea0183LOADER_H_
#define SERVER_NAUTICAL_LOGIMPORT_Nmea0183LOADER_H_

#include <iosfwd>
#include <server/nautical/logimport/LogAccumulator.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <server/nautical/logimport/SourceGroup.h>
#include <string>
#include <server/common/logging.h>
#include <server/nautical/logimport/TimeReconstructor.h>
#include <map>
#include <vector>
#include <server/common/ArrayBuilder.h>
#include <fstream>
#include <server/nautical/logimport/LogLoaderSettings.h>

namespace sail {
namespace Nmea0183Loader {

template <DataCode code>
struct GetIndexedValues;

template <typename T>
struct Filter {
  static bool accept(const T &x) {return true;}
};

template <>
struct Filter<TimeStamp> {
  static bool accept(const TimeStamp &x) {return x.defined();}
};

template <typename T>
bool acceptNmeaValue(const T &x) {
  return Filter<T>::accept(x);
}

// This function is just one possible way to make the function
// that maps the counter value to a time stamp, see comment below.
std::function<TimeStamp(int)> makeNmeaIndexToTimeFunction(
    int n, const Array<IndexedTime> &times);

// Because the Nmea0183 messages don't have timestamps,
// we assign a counter value to every
// value received. Then we estimate the time based on the
// counter values of all measurements.
// Measurements of type DATE_TIME are special,
// in the sense that they are used to
// make a mapping from counter values to time stamps.
class Nmea0183Reconstructor {
public:
  Nmea0183Reconstructor() : _index(0) {}

  template <DataCode code>
  void addValue(const typename TypeForCode<code>::type &value) {
    auto &values = GetIndexedValues<code>::apply(this);
    if (acceptNmeaValue(value)) {
      values.push_back(makeIndexed(value));
    }
  }

  #define MAKE_FIELD(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    std::vector<IndexedValue<TYPE> > _indexed##HANDLE;
  FOREACH_CHANNEL(MAKE_FIELD)
  #undef MAKE_FIELD

  bool reconstructAndOutput(
      const std::string &sourceName,
      LogAccumulator *dst,
      const TimeReconstructorSettings &settings) const {
    ArrayBuilder<IndexedTime> times;
    for (auto x: _indexedDATE_TIME) {
      times.add(x);
    }
    auto idx2time = makeNmeaIndexToTimeFunction(_index, times.get());
    if (!bool(idx2time)) {
      LOG(ERROR) << "Failed to reconstruct times in Nmea0183 stream";
      return false;
    }

#define OUTPUT_ALL_TO_ACC(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    for (auto x: _indexed##HANDLE) { \
      auto dstCh = allocateSourceIfNeeded<TYPE>(sourceName, &(dst->_##HANDLE##sources)); \
      if (isFinite(x.value)) { \
        auto time = idx2time(x.index); \
        if (time.defined()) {dstCh->push_back(TimedValue<TYPE>(time, x.value));} \
      } \
    }
FOREACH_CHANNEL(OUTPUT_ALL_TO_ACC)
#undef OUTPUT_ALL_TO_ACC
    return true;
  }
private:
  template <typename T>
  IndexedValue<T> makeIndexed(const T &x) {
    auto i = _index;
    _index++;
    return IndexedValue<T>{i, x};
  }

  int _index;
};

template <DataCode code>
struct GetIndexedValues {
  static std::vector<IndexedValue<typename TypeForCode<code>::type> >&apply(
      Nmea0183Reconstructor *x) {
    static std::vector<IndexedValue<typename TypeForCode<code>::type> > instance;
    return instance;
  }
};

#define SPECIALIZE_GET_INDEXED_VALUES(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  template <> \
  struct GetIndexedValues<HANDLE> { \
    static std::vector<IndexedValue<TYPE> > &apply(Nmea0183Reconstructor *x) { \
      return x->_indexed##HANDLE; \
    } \
  };
FOREACH_CHANNEL(SPECIALIZE_GET_INDEXED_VALUES)
#undef SPECIALIZE_GET_INDEXED_VALUES

class LogLoaderNmea0183Parser : public NmeaParser {
public:
  LogLoaderNmea0183Parser(LogAccumulator *dst,
      const std::string &s, bool ignoreWrongChecksum);
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
      NmeaParser *parser) :
    _parser(parser) {}

  template <DataCode Code>
  void add(const NmeaParser &src, const typename TypeForCode<Code>::type &value) {
    _reconstructor.addValue<Code>(value);
  }

  void setTime(const TimeStamp& time) {
    _reconstructor.addValue<DATE_TIME>(time);
  }

  void outputTo(const std::string &srcName, LogAccumulator *dst) const {
    _reconstructor.reconstructAndOutput(
        srcName, dst, TimeReconstructorSettings());
  }
 private:
  NmeaParser *_parser;
  Nmea0183Reconstructor _reconstructor;
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
    const std::string &srcName,
    const LogLoaderSettings &settings);

void loadNmea0183File(const std::string &filename, LogAccumulator *dst,
    const LogLoaderSettings &settings);


}
}

#endif /* SERVER_NAUTICAL_LOGIMPORT_Nmea0183LOADER_H_ */
