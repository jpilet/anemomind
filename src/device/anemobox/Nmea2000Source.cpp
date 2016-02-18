#include <device/anemobox/Nmea2000Source.h>


namespace sail {

using namespace PgnClasses;

namespace {
  std::string getSrc(const PgnClasses::CanPacket &c) {
    return "NMEA2000/" + c.src;
  }
}

void Nmea2000Source::process(const std::string& srcName,
                             int pgn,
                             const unsigned char* buffer,
                             int length) {
  visit(PgnClasses::CanPacket{srcName, pgn, buffer, length});
}

bool Nmea2000Source::apply(const PgnClasses::CanPacket &c, const PgnClasses::VesselHeading& packet) {
  if (!packet.valid()
      || !packet.heading().defined()) { return false; }

  _dispatcher->publishValue(
      (packet.reference().get() == VesselHeading::Reference::Magnetic ?
        MAG_HEADING : GPS_BEARING),
      getSrc(c), packet.heading().get());

  return true;
}

bool Nmea2000Source::apply(const PgnClasses::CanPacket &c, const PgnClasses::Speed& packet) {
  if (!packet.valid()) { return false; }


  if (packet.speedWaterReferenced().defined()) {
    _dispatcher->publishValue(WAT_SPEED, getSrc(c),
                              packet.speedWaterReferenced().get());
  }
  return true;
}

bool Nmea2000Source::apply(const PgnClasses::CanPacket &c, const PgnClasses::GnssPositionData& packet) {
  if (packet.valid()) {
    auto t = packet.timeStamp();
    if (t.defined()) {
      _dispatcher->publishValue(DATE_TIME, getSrc(c), t);
    }

    if (packet.longitude().defined() && packet.latitude().defined()
        && packet.altitude().defined()) {
      _dispatcher->publishValue(GPS_POS,
        getSrc(c),
        GeographicPosition<double>(packet.longitude().get(), packet.latitude().get(),
          packet.altitude().get()));
    }

    return true;
  }
  return false;
}

bool Nmea2000Source::apply(const PgnClasses::CanPacket &c, const PgnClasses::WindData& packet) {
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
    _dispatcher->publishValue(angleChannel, getSrc(c),
                              packet.windAngle().get());
  }
  if (packet.windSpeed().defined()) {
    _dispatcher->publishValue(speedChannel, getSrc(c),
                              packet.windSpeed().get());
  }
  return true;
}

bool Nmea2000Source::apply(const PgnClasses::CanPacket &c, const PgnClasses::PositionRapidUpdate& packet) {
  if (packet.valid()) {
    if (packet.longitude().defined() && packet.latitude().defined()) {
      _dispatcher->publishValue(
          GPS_POS, getSrc(c),
          GeographicPosition<double>(
              packet.longitude().get(),
              packet.latitude().get()));
      return true;
    }
  }
  return false;
}

bool Nmea2000Source::apply(const PgnClasses::CanPacket &c, const PgnClasses::CogSogRapidUpdate& packet) {
  if (packet.valid()) {
    if (packet.sog().defined()) {
      _dispatcher->publishValue(GPS_SPEED, getSrc(c), packet.sog().get());
    }
    if (packet.cog().defined() && packet.cogReference().get() == CogSogRapidUpdate::CogReference::True) {
      _dispatcher->publishValue(GPS_BEARING, getSrc(c), packet.cog().get());
    }
    return true;
  }
  return false;
}

bool Nmea2000Source::apply(const PgnClasses::CanPacket &c, const PgnClasses::TimeDate& packet) {
  if (packet.valid()) {
    auto t = packet.timeStamp();
    if (t.defined()) {
      _dispatcher->publishValue(DATE_TIME, getSrc(c), t);
      return true;
    }
  }
  return false;
}

bool Nmea2000Source::apply(const CanPacket& src, const SystemTime& packet) {
  if (packet.valid()) {
    auto t = packet.timeStamp();
    if (t.defined()) {
      _dispatcher->publishValue(DATE_TIME, getSrc(src), t);
      return true;
    }
  }
  return false;
}

bool Nmea2000Source::apply(const PgnClasses::CanPacket &c, const PgnClasses::DirectionData& packet) {
  if (packet.valid()) {
    if (packet.cog().defined()) {
      auto cog = packet.cog().get();
      if (packet.cogReference().get() == PgnClasses::DirectionData::CogReference::True) {
        _dispatcher->publishValue(GPS_BEARING, getSrc(c), cog);
      }
    }

    if (packet.speedThroughWater().defined()) {
      _dispatcher->publishValue(WAT_SPEED, getSrc(c), packet.speedThroughWater().get());
    }

    if (packet.sog().defined()) {
      _dispatcher->publishValue(GPS_SPEED, getSrc(c), packet.sog().get());
    }

    if (packet.heading().defined()) {
      _dispatcher->publishValue(MAG_HEADING/*?*/, getSrc(c), packet.heading().get());
    }

    return true;
  }
  return false;
}

}  // namespace sail

