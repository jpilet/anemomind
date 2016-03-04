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
#include <server/common/math.h>
#include <Poco/String.h>

namespace sail {

namespace {

template <typename T>
void pushBack(TimeStamp time, const T &x, typename TimedSampleCollection<T>::TimedVector *dst) {
  if (time.defined() && sail::isFinite(x)) {
    dst->push_back(TimedValue<T>(time, x));
  }
}


template <typename T>
typename TimedSampleCollection<T>::TimedVector
  *allocateSourceIfNeeded(const std::string &name, std::map<std::string, typename TimedSampleCollection<T>::TimedVector> *sources) {
  assert(sources != nullptr);
  auto found = sources->find(name);
  if (found == sources->end()) {
    auto &dst = (*sources)[name];
    return &dst;
  }
  return &(found->second);
}

struct SourcesToPopulate {
  SourcesToPopulate();
  SourcesToPopulate(const std::string &srcName, LogLoader *dst);

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

SourcesToPopulate::SourcesToPopulate() :
  awa(nullptr), aws(nullptr), externalTwa(nullptr),
  externalTws(nullptr), magHdg(nullptr), geoPos(nullptr),
  gpsBearing(nullptr), gpsSpeed(nullptr), watSpeed(nullptr) {}

SourcesToPopulate::SourcesToPopulate(const std::string &nmea0183, LogLoader *dst) :
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

void readNmeaTimePos(const NmeaParser &parser, SourcesToPopulate *toPopulate) {
  toPopulate->lastTime = parser.timestamp();
  auto t = toPopulate->lastTime;
  pushBack(t, getGpsBearing(parser), toPopulate->gpsBearing);
  pushBack(t, getGpsSpeed(parser), toPopulate->gpsSpeed);
  pushBack(t, getGeoPos(parser), toPopulate->geoPos);
}

Angle<double> getAwa(const NmeaParser &parser) {
  return Angle<double>(parser.awa());
}

Velocity<double> getAws(const NmeaParser &parser) {
  return Velocity<double>(parser.aws());
}


void readNmeaAW(const NmeaParser &parser, SourcesToPopulate *toPopulate) {
  auto t = toPopulate->lastTime;
  pushBack(t, getAwa(parser), toPopulate->awa);
  pushBack(t, getAws(parser), toPopulate->aws);
}

void readNmeaTW(const NmeaParser &parser, SourcesToPopulate *toPopulate) {
  auto t = toPopulate->lastTime;
  pushBack(t, Angle<double>(parser.twa()), toPopulate->externalTwa);
  pushBack(t, Velocity<double>(parser.tws()), toPopulate->externalTws);
}

Velocity<double> getWatSpeed(const NmeaParser &parser) {
  return Velocity<double>(parser.watSpeed());
}

Angle<double> getMagHdg(const NmeaParser &parser) {
  return Angle<double>(parser.magHdg());
}

void readNmeaWatSpHdg(const NmeaParser &parser, SourcesToPopulate *toPopulate) {
  auto t = toPopulate->lastTime;
  pushBack(t, getMagHdg(parser), toPopulate->magHdg);
  pushBack(t, getWatSpeed(parser), toPopulate->watSpeed);
}

void readNmeaVLW(const NmeaParser &parser, SourcesToPopulate *toPopulate) {
  // Ignored
}

void readNmeaData(NmeaParser::NmeaSentence s,
      const NmeaParser &parser, SourcesToPopulate *toPopulate) {
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

void parseNmea0183Char(char c, NmeaParser *parser, SourcesToPopulate *toPopulate) {
  NmeaParser::NmeaSentence s = parser->processByte(c);

    const Duration<double> maxDurationBetweenTimeMeasures
      = Duration<double>::minutes(2); // <-- TODO: Maybe soft-code this threshold in future...

    if (s != NmeaParser::NMEA_NONE) {
      readNmeaData(s, *parser, toPopulate);
    }
}

void loadNmea0183Stream(std::istream *stream, LogLoader *dst) {
  NmeaParser parser;
  parser.setIgnoreWrongChecksum(true);
  SourcesToPopulate toPopulate("NMEA0183", dst);
  while (stream->good()) {
    char c;
    stream->get(c);
    parseNmea0183Char(c, &parser, &toPopulate);
  }
}


/**
 * Loading NMEA0183 coded files
 */
void loadNmea0183File(const std::string &filename, LogLoader *dst) {
  std::ifstream file(filename);
  loadNmea0183Stream(&file, dst);
}

Angle<double> degrees = Angle<double>::degrees(1.0);
Velocity<double> knots = Velocity<double>::knots(1.0);

template <typename T>
std::function<void(std::string)> makeSetter(T unit, T *dst) {
  return [=](const std::string &s) {
    double x = 0;
    if (tryParseDouble(s, &x)) {
      *dst = x*unit;
    }
  };
}

std::function<void(std::string)> makeTimeSetter(TimeStamp *dst) {
  return [=](const std::string &s) {
    *dst = TimeStamp::parse(s);
  };
}

class CsvRowProcessor {
 public:
  CsvRowProcessor(const MDArray<std::string, 2> &header);
  void process(const MDArray<std::string, 2> &row, SourcesToPopulate *dst);
 private:
  // Prohibit copying since we have pointers pointing at this object... a bit dirty
  CsvRowProcessor &operator=(const CsvRowProcessor &other) = delete;
  CsvRowProcessor(const CsvRowProcessor &other) = delete;

  std::vector<std::function<void(std::string)> > _setters;

  Angle<double> _awa, _twa, _magHdg, _gpsBearing, _lon, _lat;
  Velocity<double> _aws, _tws, _gpsSpeed, _watSpeed;
  TimeStamp _time;

  template <typename T>
  void _pushBack(const T &x, typename TimedSampleCollection<T>::TimedVector *dst) {
    pushBack(_time, x, dst);
  }

};

void doNothing(const std::string &s) {}

CsvRowProcessor::CsvRowProcessor(const MDArray<std::string, 2> &header) {
  std::map<std::string, std::function<void(std::string)> > m;
  m["DATE/TIME(UTC)"] = makeTimeSetter(&_time);
  m["Lat."] = makeSetter(degrees, &_lat);
  m["Long."] = makeSetter(degrees, &_lon);
  m["COG"] = makeSetter(degrees, &_gpsBearing);
  m["SOG"] = makeSetter(knots, &_gpsSpeed);
  m["Heading"] = makeSetter(degrees, &_magHdg);
  m["Speed Through Water"] = makeSetter(knots, &_watSpeed);
  m["AWS"] = makeSetter(knots, &_aws);
  m["TWS"] = makeSetter(knots, &_tws);
  m["AWA"] = makeSetter(degrees, &_awa);
  m["TWA"] = makeSetter(degrees, &_twa);
  assert(header.rows() == 1);
  int cols = header.cols();
  for (int i = 0; i < cols; i++) {
    auto h = Poco::trim(header(0, i));
    auto found = m.find(h);
    bool wasFound = found != m.end();
    _setters.push_back(wasFound? found->second : doNothing);
    if (!wasFound) {
      LOG(INFO) << "CSV header ignored: '" << h << "'";
    }
  }
}

void CsvRowProcessor::process(const MDArray<std::string, 2> &row, SourcesToPopulate *dst) {
  assert(_setters.size() == row.cols());
  for (int i = 0; i < _setters.size(); i++) {
    _setters[i](row(0, i));
  }
  _pushBack(_awa, dst->awa);
  _pushBack(_aws, dst->aws);
  _pushBack(_twa, dst->externalTwa);
  _pushBack(_tws, dst->externalTws);
  _pushBack(_magHdg, dst->magHdg);
  _pushBack(_watSpeed, dst->watSpeed);
  _pushBack(_gpsSpeed, dst->gpsSpeed);
  _pushBack(_gpsBearing, dst->gpsBearing);
  auto pos = GeographicPosition<double>(_lon, _lat);

  std::cout << "Push back the geo pos: " << pos.lon().degrees()
      << ", " << pos.lat().degrees() << std::endl;
  _pushBack(pos, dst->geoPos);
  std::cout << "Now the size is " << dst->geoPos->size() << std::endl;
}

void loadCsv(const MDArray<std::string, 2> &table, LogLoader *dst) {
  SourcesToPopulate toPopulate("CSV", dst);
  CsvRowProcessor processor(table.sliceRow(0));
  int n = table.rows();
  for (int i = 1; i < n; i++) {
    processor.process(table.sliceRow(i), &toPopulate);
  }
}

void loadCsv(const std::string &filename, LogLoader *dst) {
  loadCsv(parseCsv(filename), dst);
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

void LogLoader::loadNmea0183(std::istream *s) {
  loadNmea0183Stream(s, this);
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
    loadFile(path.toString());
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
