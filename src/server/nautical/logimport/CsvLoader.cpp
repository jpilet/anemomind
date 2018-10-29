/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/logimport/CsvLoader.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/common/logging.h>
#include <Poco/String.h>
#include <server/common/CsvParser.h>
#include <server/nautical/logimport/SourceGroup.h>
#include <server/common/string.h>

#include <third_party/pstream.h>

namespace sail {

namespace {

Angle<double> degrees = Angle<double>::degrees(1.0);
Velocity<double> knots = Velocity<double>::knots(1.0);
Velocity<double> metersPerSecond = Velocity<double>::metersPerSecond(1.0);

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

void doNothing(const std::string &s) {}

std::string trimCsvString(const std::string& s) {
  std::string h = Poco::trim(s);
  if (h[0] == '"' && h.size() > 2 && h[h.size() - 1] == '"') {
    h = h.substr(1, h.size() - 2);
  }
  return h;
}

}  // namespace

class CsvRowProcessor {
 public:
  CsvRowProcessor(const MDArray<std::string, 2> &header);
  void process(const MDArray<std::string, 2> &row, SourceGroup *dst);

  bool hasValidHeader() const { return _validHeader; }
 private:
  // Prohibit copying since we have pointers pointing at this object... a bit dirty
  CsvRowProcessor &operator=(const CsvRowProcessor &other) = delete;
  CsvRowProcessor(const CsvRowProcessor &other) = delete;

  std::vector<std::function<void(std::string)> > _setters;

  Angle<double> _awa, _twa, _magHdg, _gpsBearing, _lon, _lat, _pitch, _roll;
  Velocity<double> _aws, _tws, _gpsSpeed, _watSpeed;
  TimeStamp _time;
  bool _validHeader;

  template <typename T>
  void _pushBack(const T &x, typename TimedSampleCollection<T>::TimedVector *dst) {
    pushBack(_time, x, dst);
  }

};

CsvRowProcessor::CsvRowProcessor(const MDArray<std::string, 2> &header) {
  std::map<std::string, std::function<void(std::string)> > m;
  m["DATE/TIME(UTC)"] = makeTimeSetter(&_time);
  m["Lat."] = makeSetter(degrees, &_lat);
  m["Long."] = makeSetter(degrees, &_lon);
  m["COG"] = makeSetter(degrees, &_gpsBearing);
  m["SOG"] = makeSetter(knots, &_gpsSpeed);
  m["Heading"] = makeSetter(degrees, &_magHdg);
  m["HDG"] = makeSetter(degrees, &_magHdg);
  m["Speed Through Water"] = makeSetter(knots, &_watSpeed);
  m["STW"] = makeSetter(knots, &_watSpeed);
  m["AWS"] = makeSetter(knots, &_aws);
  m["TWS"] = makeSetter(knots, &_tws);
  m["AWA"] = makeSetter(degrees, &_awa);
  m["TWA"] = makeSetter(degrees, &_twa);

  // Support for Calypso XLS recording
  m["Id"] = doNothing;
  m["Lat"] = makeSetter(degrees, &_lat);
  m["Lon"] = makeSetter(degrees, &_lon);
  m["Date"] = makeTimeSetter(&_time);
  m["AppWindAngle"] = makeSetter(degrees, &_awa);
  m["AppWindModulus"] = makeSetter(metersPerSecond, &_aws);
  m["TrueWindAngle"] = makeSetter(degrees, &_twa);
  m["TrueWindModulus"] = makeSetter(metersPerSecond, &_tws);
  m["Bearing"] = makeSetter(degrees, &_gpsBearing);
  m["Speed"] = makeSetter(metersPerSecond, &_gpsSpeed);
  m["Roll"] = makeSetter(degrees, &_roll);
  m["Pitch"] = makeSetter(degrees, &_pitch);
  m["eCompass"] = makeSetter(degrees, &_magHdg);

  assert(header.rows() == 1);
  int cols = header.cols();
  _validHeader = false;

  std::vector<std::string> ignoredHeaders;

  for (int i = 0; i < cols; i++) {
    auto h = trimCsvString(header(0, i));
    auto found = m.find(h);
    bool wasFound = found != m.end();
    _setters.push_back(wasFound? found->second : doNothing);
    _validHeader |= wasFound;

    if (!wasFound) {
      ignoredHeaders.push_back(h);
    }
  }
  if (_validHeader) {
    // It seems to be a valid CSV, but we ignored some columns.
    // It is worth notifying the user.
      LOG(INFO) << "CSV header ignored: "
        << join(ignoredHeaders, ", ");
  } else {
    // Invalid CSV. We won't read any data from this file. No need to report
    // ignored columns.
  }
}

void CsvRowProcessor::process(const MDArray<std::string, 2> &row, SourceGroup *dst) {
  assert(_setters.size() == row.cols());
  for (int i = 0; i < _setters.size(); i++) {
    _setters[i](trimCsvString(row(0, i)));
  }
  _pushBack(_awa, dst->AWA);
  _pushBack(_aws, dst->AWS);
  _pushBack(_twa, dst->TWA);
  _pushBack(_tws, dst->TWS);
  _pushBack(_magHdg, dst->MAG_HEADING);
  _pushBack(_watSpeed, dst->WAT_SPEED);
  _pushBack(_gpsSpeed, dst->GPS_SPEED);
  _pushBack(_gpsBearing, dst->GPS_BEARING);
  _pushBack(_pitch, dst->PITCH);
  _pushBack(_pitch, dst->ROLL);
  auto pos = GeographicPosition<double>(_lon, _lat);

  _pushBack(pos, dst->GPS_POS);
}

bool loadCsv(const MDArray<std::string, 2> &table,
             const std::string& source, LogAccumulator *dst) {
  if (table.rows() == 0 || table.cols() == 0) {
    return false;
  }
  SourceGroup toPopulate(source, dst);
  CsvRowProcessor processor(table.sliceRow(0));

  if (!processor.hasValidHeader()) {
    return false;
  }

  int n = table.rows();
  for (int i = 1; i < n; i++) {
    processor.process(table.sliceRow(i), &toPopulate);
  }
  return true;
}

bool loadCsv(const std::string &filename, LogAccumulator *dst) {
  std::string sourceName("CSV imported");
  return loadCsv(parseCsv(filename), sourceName, dst);
}

bool loadCsvFromPipe(const std::string& cmd, const std::string& sourceName,
                     LogAccumulator *dst) {
  redi::ipstream pipe(cmd, std::ios_base::in);
  if (!pipe.is_open()) {
    LOG(ERROR) << "Failed to run command: " << cmd;
    return false;
  }
  return loadCsv(parseCsv(&pipe), sourceName, dst);
}

} /* namespace sail */
