#include <device/anemobox/Nmea0183Source.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace sail {

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
        _dispatcher->gpsBearing()->publishValue(
            sourceName(), static_cast<Angle<double>>(_parser.gpsBearing()));
        _dispatcher->gpsSpeed()->publishValue(
            sourceName(), static_cast<Velocity<double>>(_parser.gpsSpeed()));
        _dispatcher->pos()->publishValue(sourceName(), getPos(_parser));
        _dispatcher->dateTime()->publishValue(sourceName(), getTime(_parser));
        break;
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
      case NmeaParser::NMEA_WAT_SP_HDG:
        _dispatcher->magHdg()->publishValue(
            sourceName(), static_cast<Angle<double>>(_parser.magHdg()));
        _dispatcher->watSpeed()->publishValue(
            sourceName(), static_cast<Velocity<double>>(_parser.watSpeed()));
        break;
      case NmeaParser::NMEA_VLW: break;
        _dispatcher->watDist()->publishValue(
            sourceName(), static_cast<Length<double>>(_parser.watDist()));
      case NmeaParser::NMEA_GLL:
        _dispatcher->pos()->publishValue(sourceName(), getPos(_parser));
        break;
      case NmeaParser::NMEA_VTG:
        _dispatcher->gpsBearing()->publishValue(
            sourceName(), static_cast<Angle<double>>(_parser.gpsBearing()));
        _dispatcher->gpsSpeed()->publishValue(
            sourceName(), static_cast<Velocity<double>>(_parser.gpsSpeed()));
        break;
      case NmeaParser::NMEA_ZDA:
        _dispatcher->dateTime()->publishValue(sourceName(), getTime(_parser));
        break;
    }
  }
} 

}  // namespace sail
