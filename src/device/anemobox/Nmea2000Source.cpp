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

bool Nmea2000Source::apply(const PgnClasses::GnssPositionData& packet) {
  if (packet.valid()) {
    auto t = packet.timeStamp();
    if (t.defined()) {
      _dispatcher->publishValue(DATE_TIME, _currentMessageSource, t);
    }

    if (packet.longitude().defined() && packet.latitude().defined()
        && packet.altitude().defined()) {
      _dispatcher->publishValue(GPS_POS,
        _currentMessageSource,
        GeographicPosition<double>(packet.longitude().get(), packet.latitude().get(),
          packet.altitude().get()));
    }

    return true;
  }
  return false;
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

bool Nmea2000Source::apply(const PgnClasses::PositionRapidUpdate& packet) {
  if (packet.valid()) {
    if (packet.longitude().defined() && packet.latitude().defined()) {
      _dispatcher->publishValue(
          GPS_POS, _currentMessageSource,
          GeographicPosition<double>(
              packet.longitude().get(),
              packet.latitude().get()));
      return true;
    }
  }
  return false;
}

bool Nmea2000Source::apply(const PgnClasses::CogSogRapidUpdate& packet) {
  if (packet.valid()) {
    if (packet.sog().defined()) {
      _dispatcher->publishValue(GPS_SPEED, _currentMessageSource, packet.sog().get());
    }
    if (packet.cog().defined() && packet.cogReference().get() == CogSogRapidUpdate::CogReference::True) {
      _dispatcher->publishValue(GPS_BEARING, _currentMessageSource, packet.cog().get());
    }
    return true;
  }
  return false;
}

bool Nmea2000Source::apply(const PgnClasses::TimeDate& packet) {
  if (packet.valid()) {
    auto t = packet.timeStamp();
    if (t.defined()) {
      _dispatcher->publishValue(DATE_TIME, _currentMessageSource, t);
      return true;
    }
  }
  return false;
}

bool Nmea2000Source::apply(const PgnClasses::DirectionData& packet) {
  if (packet.valid()) {
    if (packet.cog().defined()) {
      auto c = packet.cog().get();
      if (packet.cogReference().get() == PgnClasses::DirectionData::CogReference::True) {
        _dispatcher->publishValue(GPS_BEARING, _currentMessageSource, c);
      }
    }

    if (packet.speedThroughWater().defined()) {
      _dispatcher->publishValue(WAT_SPEED, _currentMessageSource, packet.speedThroughWater().get());
    }

    if (packet.sog().defined()) {
      _dispatcher->publishValue(GPS_SPEED, _currentMessageSource, packet.sog().get());
    }

    if (packet.heading().defined()) {
      _dispatcher->publishValue(MAG_HEADING/*?*/, _currentMessageSource, packet.heading().get());
    }

    return true;
  }
  return false;
}

}  // namespace sail

