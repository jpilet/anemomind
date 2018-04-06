/*
 * AstraLoader.cpp
 *
 *  Created on: 9 Mar 2018
 *      Author: jonas
 */

#include "AstraLoader.h"
#include <regex>
#include <map>
#include <iostream>
#include <server/common/string.h>
#include <server/common/logging.h>
#include <server/transducers/ParseTransducers.h>
#include <server/common/RegexUtils.h>
#include <set>
#include <fstream>
#include <server/common/Functional.h>


namespace sail {

Optional<AstraHeader> tryParseAstraHeader(const std::string& s) {
  static std::regex re("----* (.*) -*----");
  std::smatch match;
  if (std::regex_search(s, match, re) && match.size() > 1) {
    return AstraHeader{match.str(1)};
  }
  return {};
}

namespace {
}

Optional<std::map<std::string, Array<std::string>>>
  tryParseNamedNumericParameters(
    const std::string& s) {

  std::smatch m;
  using namespace Regex;
  auto notColon = "[^:]";
  auto colon = ":";
  auto wordChar = charNotInString(spaceChars + "\\:");
  auto name = captureGroup(wordChar/anyCount(notColon));
  auto value = captureGroup(basicNumber(digit));
  auto namedParameter = name/colon/anyCount(space)/value;
  static std::regex reNamedNumericParameters(
      entireString(
      anyCount(space)
        /join1(atLeastOnce(space), namedParameter)
        /anyCount(space)));
  if (std::regex_match(s, m, reNamedNumericParameters)) {
    for (int i = 0; i < m.size(); i++) {
      auto x = m[i];
      std::cout << "  * '" << x << "'" << std::endl;
      std::cout << "     At position " << m.position(i) << std::endl;
      std::cout << "     Matched? " << x.matched << std::endl;
    }
    return std::map<std::string, Array<std::string>>();
  } else {
    return {};
  }
}


namespace {
  std::pair<int, std::string> negativeLengthAndDataOf(const std::string& k) {
    return {-k.length(), k};
  }

}

// When we iterate over the possibilities
// in the parser, we want to start trying
// to match the long words (for instance we want
// to try 'DateTimeTs' before we try just 'Date'.
struct LongWordsFirst {
  bool operator()(
      const std::string& a, const std::string& b) const {
    return negativeLengthAndDataOf(a) < negativeLengthAndDataOf(b);
  }
};

template <typename Q, typename FieldAccess>
AstraValueParser inUnit(Q unit, FieldAccess f) {
  return [f, unit](const std::string& s, AstraData* dst) {
    for (auto number: tryParse<double>(s)) {
      *(f(dst)) = number*unit;
      return true;
    }
    return false;
  };
}

template <typename FieldAccess>
AstraValueParser geographicAngle(
    char posChar, char negChar,
    Angle<double> unit, FieldAccess f) {
  return [=](const std::string& s, AstraData* dst) {
    if (s.empty()) {
      return false;
    }
    char last = s.back();
    int sign = 0;
    if (last == posChar) {
      sign = 1;
    } else if (last == negChar) {
      sign = -1;
    } else {
      return false;
    }
    auto numericPart = s.substr(0, s.length()-1);

    for (auto num: tryParse<double>(numericPart)) {
      *(f(dst)) = double(sign*num)*unit;
      return true;
    }
    return false;
  };
}

template <typename FieldAccess>
AstraValueParser asString(FieldAccess f) {
  return [f](const std::string& s, AstraData* dst) {
    *(f(dst)) = s;
    return true;
  };
}

template <typename T, typename FieldAccess>
AstraValueParser asPrimitive(FieldAccess f) {
  return [f](const std::string& s, AstraData* dst) {
    for (auto x: tryParse<T>(s)) {
      *(f(dst)) = x;
      return true;
    }
    return false;
  };
}

bool ignoreAstraValue(const std::string&, AstraData*) {return true;}

Optional<Duration<double>> tryParseAstraTimeOfDay(const std::string& src) {
  return parseTimeOfDay("%T", src);
}

Optional<TimeStamp> tryParseAstraDate(const std::string& s) {
  return TimeStamp::parse("%Y/%m/%d", s);
}

Optional<AstraData> tryMakeAstraData(
    const AstraColSpec& cols,
    const AstraTableRow& rawData,
    bool verbose) {
  if (cols.size() != rawData.size()) {
    if (verbose) {
      LOG(WARNING) << "Incompatible col-spec size and raw data size";
      LOG(INFO) << "Col size: " << cols.size();
      LOG(INFO) << "Raw size: " << rawData.size();
      for (int i = 0; i < cols.size(); i++) {
        LOG(INFO) << "  Col " << i+1 << ": " << cols[i].first;
      }
      for (int i = 0; i < rawData.size(); i++) {
        LOG(INFO) << "  Data " << i+1 << ": " << rawData[i];
      }
    }
    return {};
  }
  AstraData dst;
  for (int i = 0; i < cols.size(); i++) {
    const auto& kv = cols[i];
    const auto& rd = rawData[i];
    if (!bool(kv.second)) {
      LOG(FATAL) << "No value parser registered for Astra value of type '"
          << kv.first << "'";
    }
    if (!kv.second(rd, &dst)) {
      LOG(WARNING) << "Failed to parse col '" << kv.first << "' from value '"
          << rd << "'";
      return {};
    }
  }
  return dst;
}

template <typename FieldAccess>
AstraValueParser asDate(FieldAccess f) {
  return [f](const std::string& src, AstraData* dst) {
    for (auto d: tryParseAstraDate(src)) {
      *(f(dst)) = d;
      return true;
    }
    return false;
  };
}

template <typename FieldAccess>
AstraValueParser asTimeOfDay(FieldAccess f) {
  return [f](const std::string& src, AstraData* dst) {
    for (auto p: tryParseAstraTimeOfDay(src)) {
      *(f(dst)) = p;
      return true;
    }
    return false;
  };
}

#define FIELD_ACCESS(NAME) ([](AstraData* src) {return &(src->NAME);})

/*
 *
 * WARNING: I am not at all sure about the units used
 * by Astra, e.g. degrees vs radians, m/s vs knots vs km/h.
 *
 *
 */
std::map<std::string, AstraValueParser,
  LongWordsFirst> knownHeaders{

  // Current, and things like that?
  {"Set", AstraValueParser()},
  {"Drift", AstraValueParser()},
  {"DRIFT", &ignoreAstraValue},
  {"SET", &ignoreAstraValue},

  // Wind
  {"WindType", AstraValueParser()},
  {"AWA", AstraValueParser()},
  {"AWS", AstraValueParser()},
  {"TWA", AstraValueParser()},
  {"TWS", AstraValueParser(inUnit(1.0_kn, FIELD_ACCESS(TWS)))},
  {"TWD", AstraValueParser(inUnit(1.0_deg, FIELD_ACCESS(TWD)))},
  {"GWS", AstraValueParser(inUnit(1.0_kn, FIELD_ACCESS(GWS)))},
  {"GWD", AstraValueParser(inUnit(1.0_deg, FIELD_ACCESS(GWD)))},

  // Not sure...
  {"FreezeWind", AstraValueParser()},
  {"FreezeTide", AstraValueParser()},

  // Time
  {"DateTimeTs", AstraValueParser()},
  {"Date", AstraValueParser(asDate(FIELD_ACCESS(partialTimestamp)))},
  {"Time", AstraValueParser(asTimeOfDay(FIELD_ACCESS(timeOfDay)))},
  {"Ts", &ignoreAstraValue},

  // Meta
  {"DinghyID", AstraValueParser(asPrimitive<int>(FIELD_ACCESS(dinghyId)))},
  {"UserID", AstraValueParser(asString(FIELD_ACCESS(userId)))},

  // Boat motion
  {"BS", AstraValueParser()},
  {"COG", AstraValueParser(inUnit(1.0_rad, FIELD_ACCESS(COG)))},
  {"SOG", AstraValueParser(inUnit(1.0_kn, FIELD_ACCESS(SOG)))},

  // Boat position
  {"Lat", AstraValueParser(geographicAngle(
      'N', 'S', 1.0_deg, FIELD_ACCESS(lat)))},
  {"Lon", AstraValueParser(geographicAngle(
      'E', 'W', 1.0_deg, FIELD_ACCESS(lon)))},
  {"LAT", AstraValueParser(inUnit(1.0_deg, FIELD_ACCESS(lat)))},
  {"LON", AstraValueParser(inUnit(1.0_deg, FIELD_ACCESS(lon)))},

  // Other boat orientation
  {"HDG", AstraValueParser()},
  {"Pitch", AstraValueParser(inUnit(1.0_rad, FIELD_ACCESS(pitch)))},
  {"Roll", AstraValueParser(inUnit(1.0_rad, FIELD_ACCESS(roll)))}
};

Optional<Array<std::pair<std::string, AstraValueParser>>>
  tryParseAstraColSpec(const Array<std::string>& tokens) {
  typedef std::map<std::string, AstraValueParser,
      LongWordsFirst>::iterator Iterator;

  if (tokens.empty()) {
    return {};
  }

  auto result = transduce(
      tokens,
      trMap([](const std::string& token) {
        return knownHeaders.find(token);
      })
      |
      trTakeWhile([](Iterator f){
        return f != knownHeaders.end();
      })
      |
      trMap([](Iterator f) {
        return *f;
      }),
      IntoArray<std::pair<std::string, AstraValueParser>>());
  // Only return a valid result if *all* tokens where recognized.
  return result.size() == tokens.size()?
      Optional<Array<std::pair<std::string, AstraValueParser>>>(result)
      : Optional<Array<std::pair<std::string, AstraValueParser>>>();
}

// Useful for explaining what other columns need to be considered.
void displayColHint(const AstraTableRow& rawData) {
  std::set<std::string> recognized, notRecognized;
  for (auto x: rawData) {
    auto f = knownHeaders.find(x);
    if (f == knownHeaders.end()) {
      notRecognized.insert(x);
    } else {
      recognized.insert(x);
    }
  }
  LOG(INFO) << "If this is a col spec header, then these cols are not known:";
  for (auto x: notRecognized) {
    LOG(INFO) << "  * '" << x << "'";
  }
}

bool isDinghyLogHeader(const std::string& s) {
  using namespace Regex;
  static std::regex re(entireString("Device_.*"));
  return matches(s, re);
}

bool isProcessedCoachLogHeader(const std::string& s) {
  using namespace Regex;
  static std::regex re(entireString(".*_Charts"));
  return matches(s, re);
}



Array<std::string> tokenizeAstra(const std::string& s) {
  return transduce(s, trTokenize(&isBlank), IntoArray<std::string>());
}

auto astraParser = trStreamLines() // All the lines of the file
        |
        trFilter(complementFunction(&isBlankString))
        |
        trPreparseAstraLine() // Identify the type of line: header, column spec or data?
        |
        trMakeAstraData();

Array<AstraData> loadAstraFile(const std::string& filename) {
  return transduce(
      makeOptional(std::make_shared<std::ifstream>(filename)),
      astraParser, // Produce structs from table rows.
      IntoArray<AstraData>());
}

namespace {
  std::string stringOrQ(const Optional<std::string>& s) {
    return s.defined()? s.get() : "?";
  }
  std::string stringOrQ(const Optional<int>& s) {
    std::stringstream ss;
    if (s.defined()) {
      ss << s.get();
    } else {
      ss << "?";
    }
    return ss.str();
  }

  template <typename T>
  void copyIfDefined(
      const std::string& srcName, TimeStamp full,
      const Optional<T>& x, std::map<std::string,
        typename TimedSampleCollection<T>::TimedVector>* dst) {
    if (!full.defined()) {
      LOG(WARNING) << "Missing timestamp for " << srcName;
    } else if (!x.defined()) {
      LOG(WARNING) << "Missing value";
    } else {
      (*dst)[srcName].push_back(TimedValue<T>(full, x.get()));
    }
  }

  void accumulateDinghy(const AstraData& src, LogAccumulator* dst) {
    std::string sourceName = "astraDinghy_id"
        + stringOrQ(src.dinghyId)
        + "_user"
        + stringOrQ(src.userId);

    copyIfDefined(sourceName, src.fullTimestamp(),
        src.COG, &(dst->_GPS_BEARINGsources));

    copyIfDefined(sourceName, src.fullTimestamp(),
        src.SOG, &(dst->_GPS_SPEEDsources));

    copyIfDefined(sourceName, src.fullTimestamp(),
        src.geoPos(), &(dst->_GPS_POSsources));

    // There is also Pitch, Roll
  }


  void accumulateCoach(const AstraData& src, LogAccumulator* dst) {
    std::string sourceName = "astraCoach";

    copyIfDefined(sourceName, src.fullTimestamp(),
        src.TWD, &(dst->_TWDIRsources));

    copyIfDefined(sourceName, src.fullTimestamp(),
        src.TWS, &(dst->_TWSsources));
  }
}

/**
 *
 * When loading logs, we should ensure we don't load logs from unrelated coach
 * boats that are somewhere else.
 *
 *
 */
bool accumulateAstraLogs(const std::string& filename, LogAccumulator* dst) {
  auto data = loadAstraFile(filename);
  if (data.empty()) {
    return false;
  }
  for (auto x: data) {
    switch (x.logType) {
    case AstraLogType::ProcessedCoach:
      accumulateCoach(x, dst);
      break;
    case AstraLogType::RawDinghy:
      accumulateDinghy(x, dst);
      break;
    default:
      LOG(WARNING) << "Unknown Astra log file type for "
        << filename << ". Please update the parsing code.";
      break;
    }
  }
  return true;
}



} /* namespace sail */
