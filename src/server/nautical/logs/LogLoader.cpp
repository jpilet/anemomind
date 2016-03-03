/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/logs/LogLoader.h>
#include <device/anemobox/logger/Logger.h>
#include <server/common/logging.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <Poco/Path.h>
#include <fstream>
#include <server/common/filesystem.h>
#include <server/common/CsvParser.h>

namespace sail {

namespace {
template <typename T>
typename TimedSampleCollection<T>::TimedVector
  *allocateSourceIfNeeded(std::string &name, std::map<std::string, typename TimedSampleCollection<T>::TimedVector> *sources) {
  assert(sources != nullptr);
  auto found = sources->find(name);
  if (found == sources->end()) {
    auto &dst = (*sources)[name];
    return &dst;
  }
  return &(found->second);
}

struct Nmea0183Sources {
  Nmea0183Sources();
  Nmea0183Sources(LogLoader *dst);

  TimeStamp lastTime;

  TimedSampleCollection<Angle<double> >::TimedVector *awa;
  TimedSampleCollection<Velocity<double> >::TimedVector *aws;
  TimedSampleCollection<Angle<double> >::TimedVector *externalTwa;
  TimedSampleCollection<Velocity<double> >::TimedVector *externalTws;
  TimedSampleCollection<Angle<double> >::TimedVector *magHdg;
  TimedSampleCollection<Velocity<double> >::TimedVector *watSpeed;
  TimedSampleCollection<GeographicPosition<double> >::TimedVector *geoPos;
  TimedSampleCollection<Angle<double> >::TimedVector *gpsBearing;
  TimedSampleCollection<Velocity<double> >::TimedVector *gpsSpeed;
};

Nmea0183Sources::Nmea0183Sources() :
  awa(nullptr), aws(nullptr), externalTwa(nullptr),
  externalTws(nullptr), magHdg(nullptr), geoPos(nullptr),
  gpsBearing(nullptr), gpsSpeed(nullptr), watSpeed(nullptr) {}

std::string nmea0183 = "NMEA0183";

Nmea0183Sources::Nmea0183Sources(LogLoader *dst) :
    awa(allocateSourceIfNeeded<Angle<double> >(nmea0183, dst->getAWAsources())),
    aws(allocateSourceIfNeeded<Velocity<double> >(nmea0183, dst->getAWSsources())),
    externalTwa(allocateSourceIfNeeded<Angle<double> >(nmea0183, dst->getTWAsources())),
    externalTws(allocateSourceIfNeeded<Velocity<double> >(nmea0183, dst->getTWSsources())),
    magHdg(allocateSourceIfNeeded<Angle<double> >(nmea0183, dst->getMAG_HEADINGsources())),
    watSpeed(allocateSourceIfNeeded<Velocity<double> >(nmea0183, dst->getWAT_SPEEDsources())),
    geoPos(allocateSourceIfNeeded<GeographicPosition<double> >(nmea0183, dst->getGPS_POSsources())),
    gpsBearing(allocateSourceIfNeeded<Angle<double> >(nmea0183, dst->getGPS_BEARINGsources())),
    gpsSpeed(allocateSourceIfNeeded<Velocity<double> >(nmea0183, dst->getGPS_SPEEDsources()))
  {}

Angle<double> getGpsBearing(const NmeaParser &parser) {
  return Angle<double>(parser.gpsBearing());
}

Velocity<double> getGpsSpeed(const NmeaParser &parser) {
  return Velocity<double>(parser.gpsSpeed());
}

Angle<double> getAngle(const AccAngle &x) {
  return Angle<double>::degMinMc(x.deg(), x.min(), x.mc());
}

Angle<double> getLon(const NmeaParser &parser) {
  return getAngle(parser.pos().lon);
}

Angle<double> getLat(const NmeaParser &parser) {
  return getAngle(parser.pos().lat);
}

GeographicPosition<double> getGeoPos(const NmeaParser &parser) {
  return GeographicPosition<double>(getLon(parser), getLat(parser));
}

void readNmeaTimePos(const NmeaParser &parser, Nmea0183Sources *toPopulate) {
  toPopulate->lastTime = parser.timestamp();
  toPopulate->gpsBearing->push_back(TimedValue<Angle<double> >(
      toPopulate->lastTime, getGpsBearing(parser)));

  toPopulate->gpsSpeed->push_back(TimedValue<Velocity<double> >(
      toPopulate->lastTime, getGpsSpeed(parser)));

  toPopulate->geoPos->push_back(TimedValue<GeographicPosition<double> >(
    toPopulate->lastTime, getGeoPos(parser)));
}

Angle<double> getAwa(const NmeaParser &parser) {
  return Angle<double>(parser.awa());
}

Velocity<double> getAws(const NmeaParser &parser) {
  return Velocity<double>(parser.aws());
}


void readNmeaAW(const NmeaParser &parser, Nmea0183Sources *toPopulate) {
  toPopulate->awa->push_back(TimedValue<Angle<double> >(toPopulate->lastTime, getAwa(parser)));
  toPopulate->aws->push_back(TimedValue<Velocity<double> >(toPopulate->lastTime, getAws(parser)));
}

void readNmeaTW(const NmeaParser &parser, Nmea0183Sources *toPopulate) {
  toPopulate->externalTwa->push_back(TimedValue<Angle<double> >(toPopulate->lastTime, parser.twa()));
  toPopulate->externalTws->push_back(TimedValue<Velocity<double> >(toPopulate->lastTime, parser.tws()));
}

Velocity<double> getWatSpeed(const NmeaParser &parser) {
  return Velocity<double>(parser.watSpeed());
}

Angle<double> getMagHdg(const NmeaParser &parser) {
  return Angle<double>(parser.magHdg());
}

void readNmeaWatSpHdg(const NmeaParser &parser, Nmea0183Sources *toPopulate) {
  toPopulate->magHdg->push_back(TimedValue<Angle<double>>(toPopulate->lastTime, getMagHdg(parser)));
  toPopulate->watSpeed->push_back(TimedValue<Velocity<double>>(toPopulate->lastTime, getWatSpeed(parser)));
}

void readNmeaVLW(const NmeaParser &parser, Nmea0183Sources *toPopulate) {
  // Ignored
}

void readNmeaData(NmeaParser::NmeaSentence s,
      const NmeaParser &parser, Nmea0183Sources *toPopulate) {
  switch (s) {
   case NmeaParser::NMEA_TIME_POS:
     readNmeaTimePos(parser, toPopulate);
     break;
   case NmeaParser::NMEA_AW:
     readNmeaAW(parser, toPopulate);
     break;
   case NmeaParser::NMEA_TW:
     readNmeaTW(parser, toPopulate);
     break;
   case NmeaParser::NMEA_WAT_SP_HDG:
     readNmeaWatSpHdg(parser, toPopulate);
     break;
   case NmeaParser::NMEA_VLW:
     readNmeaVLW(parser, toPopulate);
     break;
   default:
     break;
  };
}

void parseNmea0183Char(char c, NmeaParser *parser, Nmea0183Sources *toPopulate) {
  NmeaParser::NmeaSentence s = parser->processByte(c);

    const Duration<double> maxDurationBetweenTimeMeasures
      = Duration<double>::minutes(2); // <-- TODO: Maybe soft-code this threshold in future...

    if (s != NmeaParser::NMEA_NONE) {
      readNmeaData(s, *parser, toPopulate);
    }
}

/**
 * Loading NMEA0183 coded files
 */
void loadNmea0183File(const std::string &filename, LogLoader *dst) {
  std::ifstream file(filename);
  NmeaParser parser;
  parser.setIgnoreWrongChecksum(true);
  Nmea0183Sources toPopulate(dst);
  while (file.good()) {
    char c;
    file.get(c);
    parseNmea0183Char(c, &parser, &toPopulate);
  }
}






void loadCsv(const std::string &filename, LogLoader *dst) {

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

}

void LogLoader::load(const LogFile &data) {
  for (int i = 0; i < data.stream_size(); i++) {
    const auto &stream = data.stream(i);
    _sourcePriority[stream.source()] = stream.priority();

#define ADD_VALUES_TO_VECTOR(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  if (stream.shortname() == SHORTNAME) {addToVector<TYPE>(stream, &(_##HANDLE##sources[stream.source()]));}
      FOREACH_CHANNEL(ADD_VALUES_TO_VECTOR)
#undef  ADD_VALUES_TO_VECTOR

  }
}

void LogLoader::loadFile(const std::string &filename) {
  std::string ext = toLower(Poco::Path(filename).getExtension());
  if (ext == "txt") {
    loadNmea0183File(filename, this);
  } else if (ext == "csv") {
    return loadCsv(filename, this);
  } else if (ext == "log") {
    LogFile file;
    if (Logger::read(filename, &file)) {
      load(file);
    }
  } else {
    LOG(ERROR) << filename << ": unknown log file extension.";
  }
}

void LogLoader::load(const std::string &name) {
  FileTraverseSettings settings;
  settings.visitDirectories = false;
  settings.visitFiles = true;
  traverseDirectory(
      Poco::Path(name),
      [&](const Poco::Path &path) {
    loadFile(path.toString());
  }, settings);
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
