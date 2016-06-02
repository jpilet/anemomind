
#include <device/anemobox/Sources.h>

#include <server/common/logging.h>
#include <regex>

namespace sail {

namespace {

const char internalRegex[] =
  "(^NavDevice$)"
  "|(^Internal.*)"
  "|(^IMU$)"
  ;

const char externalRegex[] =
  "(^NavExternal$)"
  "|(^NMEA2000/.*)"
  "|(^NMEA0183: .*)"
  "|(^CUPS$)"
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

  LOG(WARNING)
    << "Can't classify source: "
    << source;

  return SourceOrigin::UNKNOWN;
}

bool sourceIsInternal(const std::string& source) {
  SourceOrigin origin = classify(source);
  return origin == SourceOrigin::ANEMOBOX || origin == SourceOrigin::POST_PROCESS;
}

bool sourceIsExternal(const std::string& source) {
  return !sourceIsInternal(source);
}

}  // namespace sail
