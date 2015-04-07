#include <device/anemobox/Nmea0183Source.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace sail {

bool Nmea0183Source::open(const char *path) {
  _fd = ::open(path, O_RDONLY | O_NOCTTY | O_NONBLOCK);
  return _fd >= 0;
}

void Nmea0183Source::poll() {
  if (_fd < 0) {
    return;
  }

  unsigned char buffer[256];
  ssize_t bytes = read(_fd, buffer, sizeof(buffer));

  if (bytes <= 0) {
    return;
  }

  for (ssize_t i = 0; i < bytes; ++i) {
    switch (_parser.processByte(buffer[i])) {
      case NmeaParser::NMEA_NONE:
      case NmeaParser::NMEA_UNKNOWN: break;
      case NmeaParser::NMEA_TIME_POS: break;
      case NmeaParser::NMEA_AW:
        _dispatcher->awa()->publishValue(
            sourceName(), static_cast<Angle<double>>(_parser.awa()));
        _dispatcher->aws()->publishValue(
            sourceName(), static_cast<Velocity<double>>(_parser.aws()));
        break;
      case NmeaParser::NMEA_TW:
        _dispatcher->twa()->publishValue(
            sourceName(), static_cast<Angle<double>>(_parser.twa()));
        _dispatcher->tws()->publishValue(
            sourceName(), static_cast<Velocity<double>>(_parser.tws()));
        break;
      case NmeaParser::NMEA_WAT_SP_HDG: break;
      case NmeaParser::NMEA_VLW: break;
      case NmeaParser::NMEA_GLL: break;
    }
  }
} 

}  // namespace sail
