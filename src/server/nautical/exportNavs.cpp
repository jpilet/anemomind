/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <server/common/ArgMap.h>
#include <server/common/Functional.h>
#include <server/common/TimeStamp.h>
#include <server/nautical/DownsampleGps.h>
#include <server/nautical/calib/Calibrator.h>
#include <server/nautical/logimport/LogLoader.h>
#include <sstream>

using namespace sail;
using namespace sail::NavCompat;

enum Format  {CSV, MATLAB, JSON};

struct ExportSettings {
  Format format = CSV;
  std::string formatStr;
  bool simulatedTrueWindData = false;
  bool withHeader = true;
  bool verbose = false;
  bool downSampleGps = false;
  bool preferInternalGps = false;
  std::string preferredSource;
  std::string timeFormat = "us";
};

NavDataset loadNavsFromArgs(Array<ArgMap::Arg*> args) {
  LogLoader loader;
  for (auto arg: args) {
    auto p = arg->value();
    LOG(INFO) << "Load navs from " << p;
    loader.load(p);
  }
  return loader.makeNavDataset();
}

struct NavField {
  std::string columnHeader;

  // I guess that, at least floating point and integer
  // literals look the same no matter if it is matlab, csv or json.
  // I don't think we are going to export string literals or other things like that.
  std::function<std::string(Nav)> getLiteral;
};

std::string doubleToString(double x, const ExportSettings& settings) {
  if (std::isfinite(x) || settings.format != CSV) {
    stringstream ss;
    ss.precision(std::numeric_limits<double>::max_digits10);
    ss << x;
    return ss.str();
  } else {
    return "";
  }
}

std::string angleToLiteral(Angle<double> x, const ExportSettings& settings,
                           double maxValDegrees) {
  Angle<double> maxVal = Angle<double>::degrees(maxValDegrees);
  return doubleToString(
      x.minimizeCyclicallyButNotLessThan(maxVal - Angle<double>::degrees(360)).degrees(),
      settings);
}

std::string velocityToLiteral(Velocity<double> x, const ExportSettings& settings) {
  return doubleToString(x.knots(), settings);
}

std::string timeToLiteral(TimeStamp t) {
  std::stringstream ss;
  ss << t.toMilliSecondsSince1970();
  return ss.str();
}

std::string timeToLiteralHumanReadable(TimeStamp t, const ExportSettings& settings) {
  if (settings.format == MATLAB) { // Don't export text, only numbers.
    return timeToLiteral(t);
  }
  // Round to the nearest second.
  int64_t msec = t.toMilliSecondsSince1970();
  msec = 1000 * ((msec + 500) / 1000);

  const char* timeFormat = 0;

  if (settings.timeFormat == "us") {
    timeFormat = "%m/%d/%Y %I:%M:%S %p"; // en-US
  } else if (settings.timeFormat == "fr") {
    timeFormat =  "%d.%m.%Y %T"; // FR/CH locale
  } else if (settings.timeFormat == "iso") {
    timeFormat = "%Y-%m-%dT%H:%M:%SZ";
  } else {
    LOG(FATAL) << "Unknown timeFormat: " << settings.timeFormat
      << ". Available: us, fr or iso.";
  }
  return TimeStamp::fromMilliSecondsSince1970(msec).toString(timeFormat);
}

Array<NavField> getNavFields(const ExportSettings& format) {
  ArrayBuilder<NavField> result;
  result.add(Array<NavField>{
    NavField{"DATE/TIME (UTC)", [=](const Nav &x) {
      return timeToLiteralHumanReadable(x.time(), format);
    }},
    NavField{"AWA (degrees)", [=](const Nav &x) {
      string lit = angleToLiteral(x.awa(), format, 180);
      return lit;
    }},
    NavField{"AWS (knots)", [=](const Nav &x) {
      return velocityToLiteral(x.aws(), format);
    }},
    NavField{"TWA NMEA (degrees)", [=](const Nav &x) {
      return angleToLiteral(x.externalTwa(), format, 180);
    }},
    NavField{"TWS NMEA (knots)", [=](const Nav &x) {
      return velocityToLiteral(x.externalTws(), format);
    }},
    NavField{"TWDIR NMEA (degrees)", [=](const Nav &x) {
      return angleToLiteral(x.externalTwa() + x.gpsBearing(), format, 360);
    }},
    NavField{"TWA Anemobox (degrees)", [=](const Nav &x) {
      auto angle = (x.hasDeviceTwa() ?  x.deviceTwa() : Angle<double>());
      return angleToLiteral(angle, format, 180);
    }},
    NavField{"TWS Anemobox (knots)", [=](const Nav &x) {
      auto speed = (x.hasDeviceTws() ?  x.deviceTws() : Velocity<double>());
      return velocityToLiteral(speed, format);
    }},
    NavField{"TWDIR Anemobox (degrees)", [=](const Nav &x) {
      auto twdir = (x.hasDeviceTwdir() ?  x.deviceTwdir() : Angle<double>());
      return angleToLiteral(twdir, format, 360);
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
    }},
    NavField{"Rudder angle (degrees)", [=](const Nav &x) {
      return angleToLiteral(x.rudderAngle(), format, 180);
    }}
  });

  if (format.simulatedTrueWindData) {
    result.add(Array<NavField>{
      NavField{"TWA Anemomind simulated (degrees)", [=](const Nav &x) {
        auto angle = (x.hasTrueWindOverGround() ?
            x.twaFromTrueWindOverGround()
            : Angle<double>());
        return angleToLiteral(angle, format, 180);
      }},
      NavField{"TWS Anemomind simulated (knots)", [=](const Nav &x) {
        auto speed = x.trueWindOverGround().norm();
        return velocityToLiteral(speed, format);
      }},
      NavField{"TWDIR Anemomind simulated (degrees)", [=](const Nav &x) {
        return angleToLiteral(
            x.twdir(), format, 360);
      }}
      });
  }
  return result.get();
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

std::vector<std::string> makeLiterals(
    const Array<NavField> &fields, const Nav &x) {
  std::vector<std::string> result;
  result.reserve(fields.size());

  for (int i = 0; i < fields.size(); i++) {
    result.push_back(fields[i].getLiteral(x));
  }
  return result;
}

std::string makeLiteralString(const Array<NavField> &fields,
    const Nav &x, std::string sep) {
  return join(makeLiterals(fields, x), sep);
}

int exportCsv(bool withHeader, Array<NavField> fields,
    Array<Nav> navs, std::ostream *dst) {
  if (withHeader) {
    *dst << makeHeader(fields, true) << "\n";
  }

  std::string lastTime;
  for (auto nav: navs) {
    std::vector<std::string> row{makeLiterals(fields, nav)};
    if (row[0] != lastTime) {
      lastTime = row[0];
      *dst << join(row, ",") << "\n";
    }
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

NavDataset performCalibration(NavDataset navs0,const ExportSettings& settings) {
  WindOrientedGrammarSettings gs;
  WindOrientedGrammar grammar(gs);
  auto tree = grammar.parse(navs0);
  std::shared_ptr<Calibrator> calib(new Calibrator(grammar));
  if (settings.verbose) {
    calib->setVerbose();
  }
  calib->calibrate(navs0, tree, Nav::debuggingBoatId());
  auto result = calib->simulate(navs0);
  if (result.isDefaultConstructed()) {
    LOG(WARNING) << "Failed to simulate";
    return navs0;
  }
  return result;
}

int exportNavs(Array<ArgMap::Arg*> args, const ExportSettings& settings, std::string output) {
  NavDataset navs = removeStrangeGpsPositions(loadNavsFromArgs(args));

  if (settings.preferredSource.size() > 0) {
    navs.preferSourceAll(settings.preferredSource);
  }

  if (settings.preferInternalGps) {
    navs.preferSource(
       std::set<DataCode>{GPS_POS, GPS_SPEED, GPS_BEARING}, "Internal GPS");
  }

  if (settings.downSampleGps) {
    navs = downSampleGpsTo1Hz(navs);
  }


  Array<NavField> fields = getNavFields(settings);
  if (settings.simulatedTrueWindData) {
    navs = performCalibration(navs, settings);
  }
  const std::string& format = settings.formatStr;
  LOG(INFO) << "Navs successfully loaded, export them to "
      << output << " with format " << format;
  std::ofstream file(output);

  //navs.outputSummary(&std::cout);
  std::cout << navs << endl;

  auto sampled = makeArray(navs);
  LOG(INFO) << "Number of navs to export: " << sampled.size();
  if (format == "csv") {
    return exportCsv(settings.withHeader, fields, sampled, &file);
  } else if (format == "json") {
    return exportJson(settings.withHeader, fields, sampled, &file);
  } else if (format == "matlab") {
    return exportMatlab(settings.withHeader, fields, sampled, &file);
  }
  LOG(ERROR) << ("Export format not recognized: " + format);
  return -1;
}

int main(int argc, const char **argv) {
  ExportSettings settings;
  settings.formatStr = "csv";
  std::string output = "/tmp/exported_navs.txt";

  ArgMap amap;
  amap.registerOption("--format", "What export format to use: (matlab, json, csv). Defaults to " + settings.formatStr)
      .store(&settings.formatStr);
  amap.registerOption("--output", "Where to put the exported data. Defaults to " + output)
    .store(&output);
  amap.registerOption("--no-header", "Omit header labels for data columns");
  amap.registerOption("--no-simulate", "Skip simulated true wind columns");
  amap.registerOption("-v", "Verbose output");
  amap.registerOption("-1Hz", "Downsample gps to 1Hz");
  amap.registerOption("-i", "Prefer internal GPS source");
  amap.registerOption("-s", "set default preferred source")
    .store(&settings.preferredSource);
  amap.registerOption("-t", "set time format: us, fr or iso")
    .store(&settings.timeFormat);

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
        settings.format = (settings.formatStr == "csv"?
                           CSV : (settings.formatStr == "json"? JSON : MATLAB));
        settings.withHeader = !amap.optionProvided("--no-header");
        settings.verbose = amap.optionProvided("-v");
	settings.preferInternalGps = amap.optionProvided("-i");
        settings.simulatedTrueWindData = !amap.optionProvided("--no-simulate");
        settings.downSampleGps = amap.optionProvided("-1Hz");
        return exportNavs(amap.freeArgs(), settings, output);
      }
    case ArgMap::Done:
      return 0;
    case ArgMap::Error:
      return -1;
  }
}
