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

namespace sail {

Optional<AstraHeader> tryParseAstraHeader(const std::string& s) {
  static std::regex re("----* (.*) -*----");
  std::smatch match;
  if (std::regex_search(s, match, re) && match.size() > 1) {
    return AstraHeader{match.str(1)};
  }
  return {};
}

Optional<std::map<std::string, Array<std::string>>> tryParseNamedParameters(
    const std::string& s) {
  using namespace Regex;


  auto colon = ":";
  //auto wordChar = charNotInString(spaceChars + colon);
  //auto word = atLeastOnce(wordChar);
  //auto words = join(atLeastOnce(space), word);
  //auto namedParameter = words/colon/anyCount(space)/basicNumber(digit);
  auto namedParameter = colon/basicNumber(digit);
  auto pattern = entireString(
      anyCount(space)
        /join1(atLeastOnce(space), namedParameter)
        /anyCount(space));

  std::cout << "Pattern is " << pattern << std::endl;
  static std::regex re(
      pattern);
  std::smatch m;
  if (std::regex_match(s, m, re)) {
    std::cout << "GOOD" << std::endl;
    return std::map<std::string, Array<std::string>>();
  } else {
    std::cout << "BAD" << std::endl;
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

Optional<Duration<double>> tryParseAstraTimeOfDay(const std::string& src) {
  return parseTimeOfDay("%T", src);
}

Optional<TimeStamp> tryParseAstraDate(const std::string& s) {
  return TimeStamp::parse("%Y/%m/%d", s);
}

Optional<AstraData> tryMakeAstraData(
    const AstraColSpec& cols,
    const AstraTableRow& rawData) {
  if (cols.size() != rawData.size()) {
    LOG(WARNING) << "Incompatible col-spec size and raw data size";
    for (int i = 0; i < cols.size(); i++) {
      LOG(INFO) << "  Col " << i+1 << ": " << cols[i].first;
    }
    for (int i = 0; i < rawData.size(); i++) {
      LOG(INFO) << "  Data " << i+1 << ": " << rawData[i];
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
  {"DateTimeTs", AstraValueParser()},
  {"BS", AstraValueParser()},
  {"AWA", AstraValueParser()},
  {"AWS", AstraValueParser()},
  {"HDG", AstraValueParser()},
  {"TWA", AstraValueParser()},
  {"TWS", AstraValueParser()},
  {"TWD", AstraValueParser()},
  {"WindType", AstraValueParser()},
  {"Lat", AstraValueParser()},
  {"Lon", AstraValueParser()},
  {"Set", AstraValueParser()},
  {"Drift", AstraValueParser()},
  {"FreezeWind", AstraValueParser()},
  {"FreezeTide", AstraValueParser()},
  {"Date", AstraValueParser(asDate(FIELD_ACCESS(partialTimestamp)))},
  {"Time", AstraValueParser(asTimeOfDay(FIELD_ACCESS(timeOfDay)))},
  {"DinghyID", AstraValueParser(asPrimitive<int>(FIELD_ACCESS(dinghyId)))},
  {"UserID", AstraValueParser(asString(FIELD_ACCESS(userId)))},
  {"COG", AstraValueParser(inUnit(1.0_rad, FIELD_ACCESS(COG)))},
  {"SOG", AstraValueParser(inUnit(1.0_kn, FIELD_ACCESS(SOG)))},
  {"LAT", AstraValueParser(inUnit(1.0_deg, FIELD_ACCESS(lat)))},
  {"LON", AstraValueParser(inUnit(1.0_deg, FIELD_ACCESS(lon)))},
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

Array<std::string> tokenizeAstra(const std::string& s) {
  return transduce(s, trTokenize(&isBlank), IntoArray<std::string>());
}



} /* namespace sail */
