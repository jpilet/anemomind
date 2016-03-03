/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/logs/LogLoader.h>
#include <device/anemobox/logger/Logger.h>
#include <server/common/logging.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <fstream>

namespace sail {


/*
// processGPRMC
  void readNmeaTimePos(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    dstNav->setTime(getTime(parser));
    mask->set(ParsedNavs::TIME, true);

    dstNav->setGpsBearing(getGpsBearing(parser));
    mask->set(ParsedNavs::GPS_BEARING, true);

    dstNav->setGpsSpeed(getGpsSpeed(parser));
    mask->set(ParsedNavs::GPS_SPEED);

    dstNav->setGeographicPosition(getGeoPos(parser));
    mask->set(ParsedNavs::POS);
  }

  Angle<double> getAwa(const NmeaParser &parser) {
    return Angle<double>(parser.awa());
  }

  Velocity<double> getAws(const NmeaParser &parser) {
    return Velocity<double>(parser.aws());
  }


  void readNmeaAW(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    dstNav->setAwa(getAwa(parser));
    mask->set(ParsedNavs::AWA, true);

    dstNav->setAws(getAws(parser));
    mask->set(ParsedNavs::AWS, true);
  }

  void readNmeaTW(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    dstNav->setExternalTwa(parser.twa());
    mask->set(ParsedNavs::TWA_EXTERNAL, true);
    dstNav->setExternalTws(parser.tws());
    mask->set(ParsedNavs::TWS_EXTERNAL, true);
  }

  Velocity<double> getWatSpeed(const NmeaParser &parser) {
    return Velocity<double>(parser.watSpeed());
  }

  Angle<double> getMagHdg(const NmeaParser &parser) {
    return Angle<double>(parser.magHdg());
  }

  void readNmeaWatSpHdg(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    dstNav->setWatSpeed(getWatSpeed(parser));
    mask->set(ParsedNavs::WAT_SPEED, true);

    dstNav->setMagHdg(getMagHdg(parser));
    mask->set(ParsedNavs::MAG_HDG, true);
  }

  void readNmeaVLW(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    // Ignored
  }

  void readNmeaData(NmeaParser::NmeaSentence s,
        const NmeaParser &parser) {
    switch (s) {
     case NmeaParser::NMEA_TIME_POS:
       readNmeaTimePos(parser, dstNav, mask);
       break;
     case NmeaParser::NMEA_AW:
       readNmeaAW(parser, dstNav, mask);
       break;
     case NmeaParser::NMEA_TW:
       readNmeaTW(parser, dstNav, mask);
       break;
     case NmeaParser::NMEA_WAT_SP_HDG:
       readNmeaWatSpHdg(parser, dstNav, mask);
       break;
     case NmeaParser::NMEA_VLW:
       readNmeaVLW(parser, dstNav, mask);
       break;
     default:
       break;
    };
  }
}*/

/*// TODO: move this function to somewhere that makes sense.
// and remove duplicate in Nmea0183Source.cpp
TimeStamp getTime(const NmeaParser &parser) {
  return NavDataConversion::makeTimeNmeaFromYMDhms(
      parser.year(), parser.month(), parser.day(),
      parser.hour(), parser.min(), parser.sec());
}*/


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

  TimedSampleCollection<Angle<double> >::TimedVector *awa;
  TimedSampleCollection<Velocity<double> >::TimedVector *aws;
  TimedSampleCollection<Angle<double> >::TimedVector *externalTwa;
  TimedSampleCollection<Velocity<double> >::TimedVector *externalTws;
  TimedSampleCollection<Angle<double> >::TimedVector *magHdg;
  TimedSampleCollection<GeographicPosition<double> >::TimedVector *geoPos;
  TimedSampleCollection<Angle<double> >::TimedVector *gpsBearing;
  TimedSampleCollection<Velocity<double> >::TimedVector *gpsSpeed;
};

Nmea0183Sources::Nmea0183Sources() :
  awa(nullptr), aws(nullptr), externalTwa(nullptr),
  externalTws(nullptr), magHdg(nullptr), geoPos(nullptr),
  gpsBearing(nullptr), gpsSpeed(nullptr) {}

std::string nmea0183 = "NMEA0183";

Nmea0183Sources::Nmea0183Sources(LogLoader *dst) :
  awa(allocateSourceIfNeeded<Angle<double> >(nmea0183, dst->getAWAsources())) {}



void parseNmea0183Char(char c, NmeaParser *parser) {
  NmeaParser::NmeaSentence s = parser->processByte(c);

    const Duration<double> maxDurationBetweenTimeMeasures
      = Duration<double>::minutes(2); // <-- TODO: Maybe soft-code this threshold in future...

    if (s != NmeaParser::NMEA_NONE) {
      readNmeaData(s, *parser);
    }
}

/**
 * Loading NMEA0183 coded files
 */
bool loadNmea0183File(const std::string &filename, LogLoader *dst) {
  std::ifstream file(filename);
  NmeaParser parser;
  parser.setIgnoreWrongChecksum(true);
  while (file.good()) {
    char c;
    file.get(c);
    parseNmea0183Char(c, &parser);
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

void LogLoader::loadAnyFile(const std::string &filename) {
  LogFile file;
  if (Logger::read(filename, &file)) {
    load(file);
  }
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
