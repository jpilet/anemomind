#include <device/anemobox/Nmea2000Source.h>

namespace sail {

void Nmea2000Source::process(const std::string& srcName,
                             int pgn,
                             const unsigned char* buffer,
                             int length) {
  // TODO: parse PGN and push to dispatcher.
}

}  // namespace sail

