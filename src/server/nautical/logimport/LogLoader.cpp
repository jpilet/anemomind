/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/logger/Logger.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <fstream>
#include <Poco/Path.h>
#include <Poco/String.h>
#include <server/common/CsvParser.h>
#include <server/common/filesystem.h>
#include <server/common/logging.h>
#include <server/common/math.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/logimport/CsvLoader.h>
#include <server/nautical/logimport/SourceGroup.h>
#include <device/anemobox/Nmea0183Adaptor.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/math.h>
#include <server/nautical/GeographicPosition.h>

namespace sail {

namespace { // NMEA0183
  template <typename T>
  TimeStamp updateLastTime(const TimeStamp &current, const T &candidate) {
    return current;
  }

  template <>
  TimeStamp updateLastTime(const TimeStamp &current, const TimeStamp &candidate) {
    if (!candidate.defined()) {
      return current;
    } else if (!current.defined()) {
      return candidate;
    }
    return std::max(current, candidate);
  }

  class Nmea0183LogLoaderAdaptor {
   public:
    Nmea0183LogLoaderAdaptor(NmeaParser *parser, LogLoader *dst,
        const std::string &srcName) :
      _parser(parser), _dst(dst), _sourceName(srcName) {}

    template <DataCode Code>
    void add(const std::string &sourceName, const typename TypeForCode<Code>::type &value) {
      typedef typename TypeForCode<Code>::type T;
      typedef typename TimedSampleCollection<T>::TimedVector TimedVector;
      std::map<std::string, TimedVector> *m = getChannels<Code>(_dst);
      auto dst = allocateSourceIfNeeded<T>(_sourceName, m);
      _lastTime = updateLastTime(_lastTime, value);
      if (_lastTime.defined() && isFinite(value)) {
        dst->push_back(TimedValue<T>(_lastTime, value));
      }
    }

    const std::string &sourceName() const {return _sourceName;}
   private:
    TimeStamp _lastTime;
    std::string _sourceName;
    NmeaParser *_parser;
    LogLoader *_dst;
  };


  class LogLoaderNmea0183Parser : public NmeaParser {
  public:
    LogLoaderNmea0183Parser(LogLoader *dst,
        const std::string &s) : _dst(dst), _sourceName(s) {
      setIgnoreWrongChecksum(true);
    }

    void setProtobufTime(const TimeStamp &time) {
      _protobufTime = time;
    }

  protected:
    // Well, the rudder angle is delivered by the NmeaParser
    // differently w.r.t. how it delivers the other values. Not sure
    // how to do this right.
    virtual void onXDRRudder(const char *senderAndSentence,
                             bool valid,
                             sail::Angle<double> angle,
                             const char *whichRudder) {
      auto t = latest(timestamp(), _protobufTime);
      if (valid && t.defined()) {
        (*(_dst->getRUDDER_ANGLEsources()))[_sourceName].push_back(
            TimedValue<Angle<double> >(t, angle));
      }
    }
  private:
    std::string _sourceName;
    LogLoader *_dst;
    TimeStamp _protobufTime;
  };

  std::string defaultNmea0183SourceName = "NMEA0183";

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


  void loadNmea0183Stream(std::istream *stream, LogLoader *dst,
      const std::string &srcName) {
    LogLoaderNmea0183Parser parser(dst, srcName);
    Nmea0183LogLoaderAdaptor adaptor(&parser, dst, srcName);
    streamToNmeaParser(stream, &parser, &adaptor);
  }

  void loadNmea0183File(const std::string &filename, LogLoader *dst) {
    std::ifstream file(filename);
    loadNmea0183Stream(&file, dst, defaultNmea0183SourceName);
  }
}





/**
 * Log file loading coded in our format (using protobuf)
 */
template <typename T>
void addToVector(const ValueSet &src, std::deque<TimedValue<T> > *dst) {
  std::vector<TimeStamp> timeVector;
  std::vector<T> dataVector;
  ValueSetToTypedVector<TimeStamp>::extract(src, &timeVector);
  ValueSetToTypedVector<T>::extract(src, &dataVector);
  auto n = dataVector.size();
  if (n == dataVector.size()) {
    for (size_t i = 0; i < n; i++) {
      dst->push_back(TimedValue<T>(timeVector[i], dataVector[i]));
    }
  } else {
    LOG(WARNING) << "Incompatible time and data vector sizes. Ignore this data.";
  }
}

void LogLoader::loadValueSet(const ValueSet &stream) {
#define ADD_VALUES_TO_VECTOR(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  if (stream.shortname() == SHORTNAME) {addToVector<TYPE>(stream, &(_##HANDLE##sources[stream.source()]));}
      FOREACH_CHANNEL(ADD_VALUES_TO_VECTOR)
#undef  ADD_VALUES_TO_VECTOR
  loadTextData(stream);
}

void LogLoader::loadTextData(const ValueSet &stream) {
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
    LogLoaderNmea0183Parser parser(this, dstSourceName);
    Nmea0183LogLoaderAdaptor adaptor(&parser, this, dstSourceName);
    for (int i = 0; i < n; i++) {
      parser.setProtobufTime(times[i]);
      streamToNmeaParser(stream.text(i), &parser, &adaptor);
    }
  }
}


void LogLoader::load(const LogFile &data) {
  // TODO: Define a set of standard priorities in a file somewhere
  auto rawStreamPriority = -16;


  for (int i = 0; i < data.stream_size(); i++) {
    const auto &stream = data.stream(i);
    _sourcePriority[stream.source()] = stream.priority();
    loadValueSet(stream);
  }

  for (int i = 0; i < data.text_size(); i++) {
    const auto &stream = data.text(i);
    _sourcePriority[stream.source()] = rawStreamPriority;
    loadValueSet(stream);
  }
}

bool LogLoader::loadFile(const std::string &filename) {
  std::string ext = toLower(Poco::Path(filename).getExtension());
  if (ext == "txt") {
    loadNmea0183File(filename, this);
    return true;
  } else if (ext == "csv") {
    loadCsv(filename, this);
    return true;
  } else if (ext == "log") {
    LogFile file;
    if (Logger::read(filename, &file)) {
      load(file);
      return true;
    }
    return false;
  } else {
    LOG(ERROR) << filename << ": unknown log file extension.";
    return false;
  }
}

void LogLoader::loadNmea0183(std::istream *s) {
  loadNmea0183Stream(s, this, defaultNmea0183SourceName);
}

void LogLoader::load(const std::string &name) {
  load(Poco::Path(name));
}

void LogLoader::load(const Poco::Path &name) {
  FileTraverseSettings settings;
  settings.visitDirectories = false;
  settings.visitFiles = true;
  traverseDirectory(
      name,
      [&](const Poco::Path &path) {
    std::string ext = toLower(path.getExtension());
    if (ext == "txt" || ext == "csv" || ext == "log") {
      loadFile(path.toString());
    } else {
      // Silently ignore files with unknown extensions while scanning
      // the directory
    }
  }, settings);
}

NavDataset LogLoader::loadNavDataset(const std::string &name) {
  LogLoader loader;
  loader.load(name);
  return loader.makeNavDataset();
}

NavDataset LogLoader::loadNavDataset(const Poco::Path &name) {
  LogLoader loader;
  loader.load(name);
  return loader.makeNavDataset();
}


template <typename T>
void insertValues(DataCode code,
    const std::map<std::string, typename TimedSampleCollection<T>::TimedVector> &src,
    Dispatcher *dst) {
  for (auto kv: src) {
    dst->insertValues<T>(code, kv.first, kv.second);
  }
}

void LogLoader::addToDispatcher(Dispatcher *dst) const {

  // Set the priorities first!
  for (auto kv: _sourcePriority) {
    dst->setSourcePriority(kv.first, kv.second);
  }

#define INSERT_VALUES(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    insertValues<TYPE>(HANDLE, _##HANDLE##sources, dst);
    FOREACH_CHANNEL(INSERT_VALUES);
#undef  INSERT_VALUES
}

NavDataset LogLoader::makeNavDataset() const {
  auto d = std::make_shared<Dispatcher>();
  addToDispatcher(d.get());
  return NavDataset(d);
}

}
