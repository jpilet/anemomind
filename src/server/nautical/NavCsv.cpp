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

TimeStamp tryParseRowdyTime(std::string s) {
  auto tokens = tokenize(s, " ");
  if (tokens.size() == 3) {
    auto date = tokens[0];
    auto time = tokens[1];
    auto ampm = tokens[2];
    auto monthDayYear = tokenize(date, "/");
    if (monthDayYear.size() == 3) {
      auto hourMinuteSecond = tokenize(time, ":");
      if (hourMinuteSecond.size() == 3) {
        int mdy[3] = {0, 0, 0};
        for (int i = 0; i < 3; i++) {
          if (!tryParseInt(monthDayYear[i], mdy + i)) {
            return TimeStamp();
          }
        }

        int year = mdy[2];
        int month = mdy[0];
        int day = mdy[1];

        int hour = 0, minute = 0;
        double second = 0.0;
        if (!tryParseInt(hourMinuteSecond[0], &hour)) {
          return TimeStamp();
        }
        if (!tryParseInt(hourMinuteSecond[1], &minute)) {
          return TimeStamp();
        }
        if (!tryParseDouble(hourMinuteSecond[2], &second)) {
          return TimeStamp();
        }
        int offset = (ampm == "am"? 0 : 12);
        auto ts = TimeStamp::UTC(year, month, day, hour + offset, minute, second);
        return ts;
      }
    }
  }
  return TimeStamp();
}

TimeStamp parseTime(std::string s) {
  TimeStamp ts = tryParseRowdyTime(s);
  if (ts.defined()) {
    return ts;
  }

  // TODO: Try something else.

  LOG(WARNING) << "Failed to parse time " << s;
  return TimeStamp();
}

void parseAndSetTime(std::string s, Nav *dst) {
  dst->setTime(parseTime(s));
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


Array<Nav> parse(MDArray<std::string, 2> table) {
  auto m = makeValueSetterMap();
  auto header = table.sliceRow(0);
  auto data = table.sliceRowsFrom(1);
  Array<Nav> dst(data.rows());
  for (int j = 0; j < header.cols(); j++) {
    auto h = Poco::trim(header[j]);
    if (m.find(h) != m.end()) {
      auto setter = m[h];
      for (int i = 0; i < data.rows(); i++) {
        setter(data(i, j), dst.ptr(i));
      }
    } else {
      LOG(WARNING) << "The column " << h << " was ignored.";
    }
  }
  return dst;
}

Array<Nav> parse(std::string filename) {
  return parse(CsvParser::parse(filename));
}

Array<Nav> parse(std::istream *s) {
  return parse(CsvParser::parse(s));
}


}
}
