
#include <device/anemobox/Sources.h>

#include <regex>
#include <server/common/logging.h>
#include <set>

namespace sail {

namespace {

const char internalRegex[] =
  "(^NavDevice$)"
  "|(^Internal.*)"
  "|(^IMU$)"
  "|(^Anemomind estimator$)"
  "|(^Simulated Anemomind estimator$)"
  "|(^Test.*)"
  ;

const char externalRegex[] =
  "(^NavExternal$)"
  "|(^NMEA2000/.*)"
  "|(^NMEA0183: .*)"
  "|(^NMEA0183$)"
  "|(^NMEA0183 input reparsed$)"
  "|(^CUPS$)"
  "|(^CSV imported$)"
  ;

}  // namespace

SourceOrigin classify(const std::string& source) {
  static std::regex internal(internalRegex);
  static std::regex external(externalRegex);

  if (regex_match(source, internal)) {
    return ANEMOBOX;
  }

  if (regex_match(source, external)) {
    return EXTERNAL;
  }

  // TODO: add regexp matching for server-produced sources

  static std::set<std::string> warned;

  if (warned.find(source) == warned.end()) {
    LOG(WARNING)
      << "Can't classify source: "
      << "'" + source + "'";
    warned.insert(source);
  }

  return SourceOrigin::UNKNOWN;
}

namespace {
  const char *getSourceLabel(SourceOrigin origin) {
    switch (origin) {
    case SourceOrigin::UNKNOWN:
        return "unknown";
    case SourceOrigin::POST_PROCESS:
        return "post_process";
    case SourceOrigin::ANEMOBOX:
        return "anemobox";
    case SourceOrigin::EXTERNAL:
        return "external";
    default:
        return "unknown";
    };
  }
}

std::string makeSourceName(SourceOrigin origin, const std::string& name) {
  return std::string(getSourceLabel(origin)) + "_" + name;
}

bool sourceIsInternal(const std::string& source) {
  SourceOrigin origin = classify(source);
  return origin == SourceOrigin::ANEMOBOX || origin == SourceOrigin::POST_PROCESS;
}

bool sourceIsExternal(const std::string& source) {
  return !sourceIsInternal(source);
}

}  // namespace sail
