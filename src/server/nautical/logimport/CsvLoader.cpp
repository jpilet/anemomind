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

namespace sail {

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
    static bool shown = false;
    if (!shown) {
      LOG(WARNING) << "When parsing CSV dates, fractional seconds are ignored.";
    }
    shown = true;
    *dst = TimeStamp::parse(s) - Duration<double>::hours(2.0) /* << TODO: REMOVE THIS!!!!!! */;
  };
}

class CsvRowProcessor {
 public:
  CsvRowProcessor(const MDArray<std::string, 2> &header);
  void process(const MDArray<std::string, 2> &row, SourceGroup *dst);
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

void CsvRowProcessor::process(const MDArray<std::string, 2> &row, SourceGroup *dst) {
  assert(_setters.size() == row.cols());
  for (int i = 0; i < _setters.size(); i++) {
    _setters[i](row(0, i));
  }
  _pushBack(_awa, dst->awa);
  _pushBack(_aws, dst->aws);
  _pushBack(_twa, dst->twa);
  _pushBack(_tws, dst->tws);
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

void loadCsv(const MDArray<std::string, 2> &table, LogAccumulator *dst) {
  SourceGroup toPopulate("CSV imported", dst);
  CsvRowProcessor processor(table.sliceRow(0));
  int n = table.rows();
  for (int i = 1; i < n; i++) {
    processor.process(table.sliceRow(i), &toPopulate);
  }
}

void loadCsv(const std::string &filename, LogAccumulator *dst) {
  loadCsv(parseCsv(filename), dst);
}

} /* namespace sail */
