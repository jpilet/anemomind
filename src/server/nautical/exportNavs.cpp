/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/nautical/NavNmeaScan.h>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace sail;

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

std::string doubleToString(double x) {
  stringstream ss;
  ss.precision(std::numeric_limits<double>::max_digits10);
  ss << x;
  return ss.str();
}

std::string angleToLiteral(Angle<double> x) {
  return doubleToString(x.degrees());
}

std::string velocityToLiteral(Velocity<double> x) {
  return doubleToString(x.knots());
}

std::string timeToLiteral(TimeStamp t) {
  std::stringstream ss;
  ss << t.toMilliSecondsSince1970();
  return ss.str();
}

Array<NavField> getNavFields() {
  return Array<NavField>{
    NavField{"Epoch (milliseconds since 1970)", [&](const Nav &x) {
      return timeToLiteral(x.time());
    }},
    NavField{"AWA (degrees)", [&](const Nav &x) {
      return angleToLiteral(x.awa());
    }},
    NavField{"AWS (knots)", [&](const Nav &x) {
      return velocityToLiteral(x.aws());
    }},
    NavField{"MagHdg (degrees)", [&](const Nav &x) {
      return angleToLiteral(x.magHdg());
    }},
    NavField{"Wat speed (knots)", [&](const Nav &x) {
      return velocityToLiteral(x.watSpeed());
    }},
    NavField{"Longitude (degrees)", [&](const Nav &x) {
      return angleToLiteral(x.geographicPosition().lon());
    }},
    NavField{"Latitude (degree)", [&](const Nav &x) {
      return angleToLiteral(x.geographicPosition().lat());
    }},
    NavField{"GPS speed (knots)", [&](const Nav &x) {
      return velocityToLiteral(x.gpsSpeed());
    }},
    NavField{"GPS bearing (degrees)", [&](const Nav &x) {
      return angleToLiteral(x.gpsBearing());
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
  Array<NavField> fields = getNavFields();
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
  switch (amap.parse(argc, argv)) {
    case ArgMap::Continue:
      return exportNavs(!amap.optionProvided("--no-header"),
          amap.freeArgs(), format, output);
    case ArgMap::Done:
      return 0;
    case ArgMap::Error:
      return -1;
  }
}
