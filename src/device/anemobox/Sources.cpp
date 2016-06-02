
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

enum SourceOrigin { INTERNAL, EXTERNAL, UNKNOWN };

SourceOrigin classify(const std::string& source) {
  static std::regex internal(internalRegex);
  static std::regex external(externalRegex);

  if (regex_match(source, internal)) {
    return INTERNAL;
  }

  if (regex_match(source, external)) {
    return EXTERNAL;
  }

  LOG(WARNING)
    << "Can't classify source as ext. or internal: "
    << source;

  // Unknown sources default to EXTERNAL
  return SourceOrigin::EXTERNAL;
}


}  // namespace

bool sourceIsInternal(const std::string& source) {
  return classify(source) == SourceOrigin::INTERNAL;
}

bool sourceIsExternal(const std::string& source) {
  return classify(source) == SourceOrigin::EXTERNAL;
}

}  // namespace sail
