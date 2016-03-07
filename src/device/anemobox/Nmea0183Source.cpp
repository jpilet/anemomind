#include <device/anemobox/Nmea0183Source.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace sail {

// TODO: move this function to somewhere that makes sense.
// and remove duplicate in NavNmea.cpp
TimeStamp getTime(const NmeaParser& parser) {
  return parser.timestamp();
}

GeographicPosition<double> getPos(const NmeaParser& parser) {
  return GeographicPosition<double>(
      Angle<double>::degrees(parser.pos().lon.toDouble()),
      Angle<double>::degrees(parser.pos().lat.toDouble()));
}

void Nmea0183Source::process(const unsigned char* buffer, int length) {
  for (ssize_t i = 0; i < length; ++i) {
    switch (_parser.processByte(buffer[i])) {
      case NmeaParser::NMEA_NONE:
      case NmeaParser::NMEA_UNKNOWN: break;
      case NmeaParser::NMEA_RMC:
        _dispatcher->publishValue(GPS_BEARING, sourceName(),
                                  static_cast<Angle<double>>(_parser.gpsBearing()));
        _dispatcher->publishValue(GPS_SPEED, sourceName(),
                                  static_cast<Velocity<double>>(_parser.gpsSpeed()));
        _dispatcher->publishValue(GPS_POS, sourceName(), getPos(_parser));
        _dispatcher->publishValue(DATE_TIME, sourceName(), getTime(_parser));
        break;
      case NmeaParser::NMEA_AW:
        _dispatcher->publishValue(AWA,
            sourceName(), static_cast<Angle<double>>(_parser.awa()));
        _dispatcher->publishValue(AWS,
            sourceName(), static_cast<Velocity<double>>(_parser.aws()));
        break;
      case NmeaParser::NMEA_TW:
        _dispatcher->publishValue(TWA,
            sourceName(), static_cast<Angle<double>>(_parser.twa()));
        _dispatcher->publishValue(TWS,
            sourceName(), static_cast<Velocity<double>>(_parser.tws()));
        break;
      case NmeaParser::NMEA_WAT_SP_HDG:
        _dispatcher->publishValue(MAG_HEADING,
            sourceName(), static_cast<Angle<double>>(_parser.magHdg()));
        _dispatcher->publishValue(WAT_SPEED,
            sourceName(), static_cast<Velocity<double>>(_parser.watSpeed()));
        break;
      case NmeaParser::NMEA_VLW: break;
        _dispatcher->publishValue(WAT_DIST,
            sourceName(), static_cast<Length<double>>(_parser.watDist()));
      case NmeaParser::NMEA_GLL:
        _dispatcher->publishValue(GPS_POS, sourceName(), getPos(_parser));
        break;
      case NmeaParser::NMEA_VTG:
        _dispatcher->publishValue(GPS_BEARING,
            sourceName(), static_cast<Angle<double>>(_parser.gpsBearing()));
        _dispatcher->publishValue(GPS_SPEED,
            sourceName(), static_cast<Velocity<double>>(_parser.gpsSpeed()));
        break;
      case NmeaParser::NMEA_ZDA:
        _dispatcher->publishValue(DATE_TIME, sourceName(), getTime(_parser));
        break;
    }
  }
} 

}  // namespace sail
