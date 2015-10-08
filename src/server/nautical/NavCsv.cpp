/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/NavCsv.h>
#include <server/common/CsvParser.h>
#include <Poco/DateTimeParser.h>
#include <Poco/Exception.h>
#include <iostream>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <Poco/String.h>

namespace sail {
namespace NavCsv {

typedef std::function<void(std::string,Nav*)> ValueSetter;

ValueSetter makeDoubleSetter(std::function<void(double, Nav*)> setter) {
  return [=](std::string s, Nav *dst) {
    double x = 0;
    if (tryParseDouble(s, &x)) {
      setter(x, dst);
    }
  };
}

namespace {
  Angle<double> degrees = Angle<double>::degrees(1.0);
  Velocity<double> knots = Velocity<double>::knots(1.0);
}





void parseAndSetTime(std::string s, Nav *dst) {
  dst->setTime(TimeStamp::parse(s));
}


std::map<std::string, ValueSetter> makeValueSetterMap() {
  std::map<std::string, ValueSetter> m;

  m["DATE/TIME(UTC)"] = &parseAndSetTime;

  m["Lat."] = makeDoubleSetter([](double x, Nav *dst) {
    dst->geographicPosition().setLat(x*degrees);
  });
  m["Long."] = makeDoubleSetter([](double x, Nav *dst) {
    dst->geographicPosition().setLon(x*degrees);
  });
  m["COG"] = makeDoubleSetter([](double x, Nav *dst) {
    dst->setGpsBearing(x*degrees);
  });
  m["SOG"] = makeDoubleSetter([](double x, Nav *dst) {
    dst->setGpsSpeed(x*knots);
  });
  m["Heading"] = makeDoubleSetter([](double x, Nav *dst) {
    dst->setMagHdg(x*degrees);
  });
  m["Speed Through Water"] = makeDoubleSetter([](double x, Nav *dst) {
    dst->setWatSpeed(x*knots);
  });
  m["AWS"] = makeDoubleSetter([](double x, Nav *dst) {
    dst->setAws(x*knots);
  });
  m["TWS"] = makeDoubleSetter([](double x, Nav *dst) {
    dst->setExternalTws(x*knots);
  });
  m["AWA"] = makeDoubleSetter([](double x, Nav *dst) {
    dst->setAwa(x*degrees);
  });
  m["TWA"] = makeDoubleSetter([](double x, Nav *dst) {
    dst->setExternalTwa(x*degrees);
  });
  return m;
}

std::string addNonEmpty(std::string total, std::string toAdd) {
  if (toAdd.empty()) {
    return total;
  } else if (total.empty()) {
    return toAdd;
  }
  return total + ", " + toAdd;
}



std::string processColumn(std::map<std::string, ValueSetter> &m,
    Array<Nav> *dst, MDArray<std::string, 2> col) {
  auto h = Poco::trim(col(0, 0));
  auto data = col.sliceRowsFrom(1);
  if (m.find(h) != m.end()) {
    auto setter = m[h];
    for (int i = 0; i < data.rows(); i++) {
      setter(data(i, 0), dst->ptr(i));
    }
    return "";
  } else {
    return h;
  }
}

Array<Nav> parse(MDArray<std::string, 2> table) {
  auto m = makeValueSetterMap();
  auto data = table.sliceRowsFrom(1);
  Array<Nav> dst(data.rows());
  std::string ignoredColumns = "";
  for (int j = 0; j < table.cols(); j++) {
    ignoredColumns = addNonEmpty(ignoredColumns, processColumn(m, &dst, table.sliceCol(j)));
  }
  if (!ignoredColumns.empty()) {
    LOG(WARNING) << "The following columns were ignored: " << ignoredColumns;
  }
  return dst;
}

Array<Nav> parse(std::string filename) {
  return parse(parseCsv(filename));
}

Array<Nav> parse(std::istream *s) {
  return parse(parseCsv(s));
}


}
}
