/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/nautical/NavNmeaScan.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <server/common/TimeStamp.h>
#include <iostream>

using namespace sail;

enum Format  {CSV, MATLAB, JSON};

Array<Nav> loadNavsFromArgs(Array<ArgMap::Arg*> args) {
  auto allNavs = args.map<Array<Nav> >([&](ArgMap::Arg *arg) {
    auto p = arg->value();
    LOG(INFO) << "Load navs from " << p;
    return scanNmeaFolder(p, Nav::debuggingBoatId());
  });
  return concat(allNavs);
}

struct NavField {
  std::string columnHeader;

  // I guess that, at least floating point and integer
  // literals look the same no matter if it is matlab, csv or json.
  // I don't think we are going to export string literals or other things like that.
  std::function<std::string(Nav)> getLiteral;
};

std::string doubleToString(double x, Format f) {
  if (std::isfinite(x) || f != CSV) {
    stringstream ss;
    ss.precision(std::numeric_limits<double>::max_digits10);
    ss << x;
    return ss.str();
  } else {
    return "";
  }
}

std::string angleToLiteral(Angle<double> x, Format f, double maxValDegrees) {
  Angle<double> maxVal = Angle<double>::degrees(maxValDegrees);
  return doubleToString(
      x.moveToInterval(maxVal - Angle<double>::degrees(360), maxVal).degrees(),
      f);
}

std::string velocityToLiteral(Velocity<double> x, Format f) {
  return doubleToString(x.knots(), f);
}

std::string timeToLiteralHumanReadable(TimeStamp t, Format f) {
  if (f == MATLAB) { // Don't export text, only numbers.
    return "NAN";
  }
  return t.toString("%m-%d-%Y %T");
}


std::string timeToLiteral(TimeStamp t) {
  std::stringstream ss;
  ss << t.toMilliSecondsSince1970();
  return ss.str();
}

Angle<double> twa(const Nav& nav) {
  return (nav.hasTrueWindOverGround() ?
          nav.trueWindOverGround().angle()
          : nav.externalTwa());
}

Array<NavField> getNavFields(std::string f) {
  auto format = (f == "csv"? CSV : (f == "json"? JSON : MATLAB));
  return Array<NavField>{
    NavField{"DATE/TIME (UTC)", [=](const Nav &x) {
      return timeToLiteralHumanReadable(x.time(), format);
    }},
    NavField{"AWA (degrees)", [=](const Nav &x) {
      return angleToLiteral(x.awa(), format, 180);
    }},
    NavField{"AWS (knots)", [=](const Nav &x) {
      return velocityToLiteral(x.aws(), format);
    }},
    NavField{"TWA (degrees)", [=](const Nav &x) {
      return angleToLiteral(twa(x), format, 180);
    }},
    NavField{"TWS (knots)", [=](const Nav &x) {
      auto speed = (x.hasTrueWindOverGround() ?
                    x.trueWindOverGround().norm()
                    : x.externalTws());
      return velocityToLiteral(speed, format);
    }},
    NavField{"TWDIR (degrees)", [=](const Nav &x) {
      return angleToLiteral(twa(x) + x.gpsBearing(), format, 360);
    }},
    NavField{"MagHdg (degrees)", [=](const Nav &x) {
      return angleToLiteral(x.magHdg(), format, 360);
    }},
    NavField{"Wat speed (knots)", [=](const Nav &x) {
      return velocityToLiteral(x.watSpeed(), format);
    }},
    NavField{"Longitude (degrees)", [=](const Nav &x) {
      return angleToLiteral(x.geographicPosition().lon(), format, 180);
    }},
    NavField{"Latitude (degree)", [=](const Nav &x) {
      return angleToLiteral(x.geographicPosition().lat(), format, 180);
    }},
    NavField{"GPS speed (knots)", [=](const Nav &x) {
      return velocityToLiteral(x.gpsSpeed(), format);
    }},
    NavField{"GPS bearing (degrees)", [=](const Nav &x) {
      return angleToLiteral(x.gpsBearing(), format, 360);
    }}
  };
}


std::string quote(const std::string &x) {
  return "\"" + x + "\"";
}

std::string makeHeader(Array<NavField> fields, bool quoted) {
  std::string result;
  int last = fields.size()-1;
  for (int i = 0; i < fields.size(); i++) {
    auto h = fields[i].columnHeader;
    result += (quoted? quote(h) : h) + (i == last? "" : ", ");
  }
  return result;
}

std::string makeLiteralString(const Array<NavField> &fields,
    const Nav &x, std::string sep) {
  std::string result;
  int last = fields.size() - 1;
  for (int i = 0; i < fields.size(); i++) {
    auto v = fields[i].getLiteral(x);
    result += v + (i == last? "" : sep);
  }
  return result;
}

int exportCsv(bool withHeader, Array<NavField> fields,
    Array<Nav> navs, std::ostream *dst) {
  if (withHeader) {
    *dst << makeHeader(fields, true) << "\n";
  }
  for (auto nav: navs) {
    *dst << makeLiteralString(fields, nav, ", ") << "\n";
  }
  return 0;
}

int exportJson(bool withHeader, Array<NavField> fields, Array<Nav> navs,
    std::ostream *dst) {
  *dst << "[";
  if (withHeader) {
    *dst << "[" << makeHeader(fields, true) << "],\n";
  }
  int last = navs.size() - 1;
  for (int i = 0; i < navs.size(); i++) {
    *dst << "[" << makeLiteralString(fields, navs[i], ", ") << "]" << (i == last? "" : ", ");
  }
  *dst << "]";
  return 0;
}

int exportMatlab(bool withHeader, Array<NavField> fields,
    Array<Nav> navs, std::ostream *dst) {
  if (withHeader) {
    *dst << "% Columns: " << makeHeader(fields, false) << "\n";
  }
  for (auto nav: navs) {
    *dst << makeLiteralString(fields, nav, " ") << "\n";
  }
  return 0;
}

int exportNavs(bool withHeader, Array<ArgMap::Arg*> args, std::string format, std::string output) {
  Array<Nav> navs = loadNavsFromArgs(args);
  Array<NavField> fields = getNavFields(format);
  std::sort(navs.begin(), navs.end());
  if (navs.empty()) {
    LOG(ERROR) << "No navs were loaded";
    return -1;
  }
  LOG(INFO) << "Navs successfully loaded, export them to "
      << output << " with format " << format;
  std::ofstream file(output);
  if (format == "csv") {
    return exportCsv(withHeader, fields, navs, &file);
  } else if (format == "json") {
    return exportJson(withHeader, fields, navs, &file);
  } else if (format == "matlab") {
    return exportMatlab(withHeader, fields, navs, &file);
  }
  LOG(ERROR) << ("Export format not recognized: " + format);
  return -1;
}

int main(int argc, const char **argv) {
  std::string format = "csv";
  std::string output = "/tmp/exported_navs.txt";

  ArgMap amap;
  amap.registerOption("--format", "What export format to use: (matlab, json, csv). Defaults to " + format)
      .store(&format);
  amap.registerOption("--output", "Where to put the exported data. Defaults to " + output)
    .store(&output);
  amap.registerOption("--no-header", "Omit header labels for data columns");
  amap.setHelpInfo(
      std::string("") +
      "Exports nav data to other formats. In addition to the named arguments,\n" +
      "it also accepts any number of free arguments pointing to directories\n" +
      "where navs are located. Example usage: \n" +
      "./nautical_exportNavs ~/sailsmart/datasets/Irene --output /tmp/irene_data.txt --format matlab\n");
  switch (amap.parse(argc, argv)) {
    case ArgMap::Continue:
      if (amap.freeArgs().empty()) {
        std::cout << "No folder provided.\n";
        amap.dispHelp(&std::cout);
        return 0;
      } else {
        return exportNavs(!amap.optionProvided("--no-header"),
            amap.freeArgs(), format, output);
      }
    case ArgMap::Done:
      return 0;
    case ArgMap::Error:
      return -1;
  }
}
