#include <device/anemobox/Nmea2000Source.h>


namespace sail {

using namespace PgnClasses;

void Nmea2000Source::process(const std::string& srcName,
                             int pgn,
                             const unsigned char* buffer,
                             int length) {
  _currentMessageSource = "NMEA2000/" + srcName;
  visit(pgn, buffer, length);
}

bool Nmea2000Source::apply(const PgnClasses::VesselHeading& packet) {
  if (!packet.valid()
      || !packet.heading().defined()) { return false; }

  _dispatcher->publishValue(
      (packet.reference().get() == VesselHeading::Reference::Magnetic ?
        MAG_HEADING : GPS_BEARING),
      _currentMessageSource, packet.heading().get());

  return true;
}

bool Nmea2000Source::apply(const PgnClasses::Speed& packet) {
  if (!packet.valid()) { return false; }


  if (packet.speedWaterReferenced().defined()) {
    _dispatcher->publishValue(WAT_SPEED, _currentMessageSource,
                              packet.speedWaterReferenced().get());
  }
  return true;
}

bool Nmea2000Source::apply(const PgnClasses::WindData& packet) {
  if (!packet.valid()) { return false; }

  DataCode angleChannel;
  DataCode speedChannel;
  switch (packet.reference().get()) {
    case WindData::Reference::Apparent:
      angleChannel = AWA;
      speedChannel = AWS;
      break;
    case WindData::Reference::True_ground_referenced_to_North:
      angleChannel = TWA;
      speedChannel = TWS;
      break;
    default:
      return false;
  }

  if (packet.windAngle().defined()) {
    _dispatcher->publishValue(angleChannel, _currentMessageSource,
                              packet.windAngle().get());
  }
  if (packet.windSpeed().defined()) {
    _dispatcher->publishValue(speedChannel, _currentMessageSource,
                              packet.windSpeed().get());
  }
  return true;
}

}  // namespace sail

