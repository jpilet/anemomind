#include <device/anemobox/Nmea2000Source.h>


namespace sail {

using namespace PgnClasses;

namespace {
  std::string makeDispatcherSourceName(uint64_t x) {
    std::stringstream ss;
    ss << "NMEA2000/" << std::hex << x;
    return ss.str();
  }
}

Nmea2000Source::Nmea2000Source(
    tNMEA2000* source,
    Dispatcher *dispatcher)
  : _deviceList(
      source == nullptr?
          nullptr
          : new tN2kDeviceList(source)),
    _dispatcher(dispatcher) {
  if (source == nullptr) {
    LOG(WARNING) << "The tNMEA2000 instance is null. "
        "Probably you only want that in a unit test.";
  }
}


Optional<uint64_t> Nmea2000Source::getSourceName(uint8_t shortName) {
  auto device = (_deviceList?
      _deviceList->FindDeviceBySource(shortName)
      : nullptr);
  return device == nullptr?
      Optional<uint64_t>()
      : Optional<uint64_t>(device->GetName());
}

std::string deviceNameToString(const Optional<uint64_t>& dn) {
  return dn.defined()?
      makeDispatcherSourceName(dn.get())
      : "UndefinedNMEA2000Source";
}


void Nmea2000Source::process(
    const tN2kMsg& msg) {
  _lastSourceName = deviceNameToString(getSourceName(msg.Source));
  visit(msg);
}

bool Nmea2000Source::apply(const tN2kMsg &c, const PgnClasses::VesselHeading& packet) {
  if (!packet.hasSomeData()
      || !packet.heading.defined()) { return false; }

  _dispatcher->publishValue(
      (packet.reference.get() == VesselHeading::Reference::Magnetic ?
        MAG_HEADING : GPS_BEARING),
      _lastSourceName, packet.heading.get());

  return true;
}

bool Nmea2000Source::apply(const tN2kMsg &c, const PgnClasses::Speed& packet) {
  if (!packet.hasSomeData()) { return false; }


  if (packet.speedWaterReferenced.defined()) {
    _dispatcher->publishValue(WAT_SPEED, _lastSourceName,
                              packet.speedWaterReferenced.get());
  }
  return true;
}

bool Nmea2000Source::apply(const tN2kMsg &c, const PgnClasses::GnssPositionData& packet) {
  if (packet.hasSomeData()) {
    auto t = packet.timeStamp();
    if (t.defined()) {
      _dispatcher->publishValue(DATE_TIME, _lastSourceName, t);
    }

    if (packet.longitude.defined() && packet.latitude.defined()
        && packet.altitude.defined()) {
      _dispatcher->publishValue(GPS_POS,
        _lastSourceName,
        GeographicPosition<double>(
            packet.longitude.get(), packet.latitude.get(),
          packet.altitude.get()));
    }

    return true;
  }
  return false;
}

bool Nmea2000Source::apply(const tN2kMsg &c, const PgnClasses::WindData& packet) {
  if (!packet.hasSomeData()) { return false; }

  DataCode angleChannel;
  DataCode speedChannel;
  switch (packet.reference.get()) {
    case WindData::Reference::Apparent:
      angleChannel = AWA;
      speedChannel = AWS;
      break;
    case WindData::Reference::True_boat_referenced:
      angleChannel = TWA;
      speedChannel = TWS;
      break;
    case WindData::Reference::True_ground_referenced_to_North:
      angleChannel = TWDIR;
      speedChannel = TWS;
      break;
    default:
      return false;
  }

  if (packet.windAngle.defined()) {
    _dispatcher->publishValue(angleChannel, _lastSourceName,
                              packet.windAngle.get());
  }
  if (packet.windSpeed.defined()) {
    _dispatcher->publishValue(speedChannel, _lastSourceName,
                              packet.windSpeed.get());
  }
  return true;
}

bool Nmea2000Source::apply(const tN2kMsg &c, const PgnClasses::PositionRapidUpdate& packet) {
  if (packet.hasSomeData()) {
    if (packet.longitude.defined() && packet.latitude.defined()) {
      _dispatcher->publishValue(
          GPS_POS, _lastSourceName,
          GeographicPosition<double>(
              packet.longitude.get(),
              packet.latitude.get()));
      return true;
    }
  }
  return false;
}

bool Nmea2000Source::apply(const tN2kMsg &c, const PgnClasses::CogSogRapidUpdate& packet) {
  if (packet.hasSomeData()) {
    if (packet.sog.defined()) {
      _dispatcher->publishValue(GPS_SPEED, _lastSourceName, packet.sog.get());
    }
    if (packet.cog.defined() && packet.cogReference.get() == CogSogRapidUpdate::CogReference::True) {
      _dispatcher->publishValue(GPS_BEARING, _lastSourceName, packet.cog.get());
    }
    return true;
  }
  return false;
}

bool Nmea2000Source::apply(const tN2kMsg &c, const PgnClasses::TimeDate& packet) {
  if (packet.hasSomeData()) {
    auto t = packet.timeStamp();
    if (t.defined()) {
      _dispatcher->publishValue(DATE_TIME, _lastSourceName, t);
      return true;
    }
  }
  return false;
}

bool Nmea2000Source::apply(const tN2kMsg& src, const SystemTime& packet) {
  if (packet.hasSomeData()) {
    auto t = packet.timeStamp();
    if (t.defined()) {
      _dispatcher->publishValue(DATE_TIME, _lastSourceName, t);
      return true;
    }
  }
  return false;
}

bool Nmea2000Source::apply(const tN2kMsg &c, const PgnClasses::DirectionData& packet) {
  if (packet.hasSomeData()) {
    if (packet.cog.defined()) {
      auto cog = packet.cog.get();
      if (packet.cogReference.get() == PgnClasses::DirectionData::CogReference::True) {
        _dispatcher->publishValue(GPS_BEARING, _lastSourceName, cog);
      }
    }

    if (packet.speedThroughWater.defined()) {
      _dispatcher->publishValue(WAT_SPEED, _lastSourceName, packet.speedThroughWater.get());
    }

    if (packet.sog.defined()) {
      _dispatcher->publishValue(GPS_SPEED, _lastSourceName, packet.sog.get());
    }

    if (packet.heading.defined()) {
      _dispatcher->publishValue(MAG_HEADING/*?*/, _lastSourceName, packet.heading.get());
    }

    return true;
  }
  return false;
}

bool Nmea2000Source::apply(const tN2kMsg &c,
                           const PgnClasses::Rudder& packet) {
  if (packet.hasSomeData() && packet.instance.defined()) {
    if (packet.position.defined()) {
      std::string source = _lastSourceName + " i" + std::to_string(packet.instance.get());
      _dispatcher->publishValue(
          RUDDER_ANGLE, source, packet.position.get());
    }
    return true;
  }
  return false;
}

}  // namespace sail

