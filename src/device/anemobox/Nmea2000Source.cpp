#include <device/anemobox/Nmea2000Source.h>
#include <device/anemobox/Nmea2000Utils.h>
#include <server/common/Result.h>

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
  : 
    tNMEA2000::tMsgHandler(0, source),
    _deviceList(source? new tN2kDeviceList(source) : nullptr),
    _dispatcher(dispatcher) {
  if (source == nullptr) {
    LOG(WARNING)
        << "You may have forgotten to "
            "provide a tNMEA2000 instance";
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

Nmea2000Source::SendOptions::SendOptions()
  : priority(N2kMsgBuilder::defaultPriority),
    destination(0xFF) {} // <-- What to put here?

bool Nmea2000Source::send(
    int sourceDeviceIndex,
    const PgnClasses::PgnBaseClass& msg,
    const SendOptions& opts) {
  N2kMsgBuilder b;
  b.PGN = msg.code();
  b.priority = opts.priority;
  b.destination = opts.destination;

  // Overwritten by SendMsg iff 0 <= sourceDeviceIndex
  b.source = 0xFF;

  auto n2k = this->GetNMEA2000(); // From the tMsgHandler base
  CHECK(n2k != nullptr);

  return n2k->SendMsg(
      b.make(msg.encode()),
      sourceDeviceIndex);
}

std::string deviceNameToString(const Optional<uint64_t>& dn) {
  return dn.defined()?
      makeDispatcherSourceName(dn.get())
      : "UndefinedNMEA2000Source";
}


void Nmea2000Source::HandleMsg(
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


// A function that tries to send a message
// of tagged value to an Nmea2000Source and
// returns a result. If the result is not
// a success, then the function must not have
// modified the Nmea2000Source.
typedef Nmea2000Source::Result
    (*TaggedValueSender)(
        std::map<std::string, TaggedValue>,
        Nmea2000Source*);

Nmea2000Source::Result orError(
    const Nmea2000Source::Result& a,
    const Nmea2000Source::Result& b) {
  if (!a.success) {
    return a;
  }
  return b;
}



Nmea2000Source::Result extractTYped

template <typename T>
Nmea2000Source::Result extract(
    const std::map<std::string, TaggedValue>& src,
    const char* key,
    T* dst) {
  Nmea2000Source::Result r;

  auto f = src.find(key);
  if (f == src.end()) {
    r.success = false;
    r.explanation += "Missing key '" + key + "'";
    return r;
  }

  return extractTypedValue(f->second, dst);
}

Nmea2000Source::Result sendPositionRapidUpdate(
    const std::map<std::string, TaggedValue>& src,
    Nmea2000Source* dst) {
  Angle<double> lon = 0.0_rad;
  Angle<double> lat = 0.0_rad;
  auto r = orError(
      extract(src, "lon", &lon),
      extract(src, "lat", &lat));
  if (!r.success) {
    return r;
  }



  return r;
}

Nmea2000Source::Result 
Nmea2000Source::send(
    const std::map<std::string, TaggedValue>& src) {
  static std::vector<TaggedValueSender> senders{
    &sendPositionRapidUpdate
  };
  for (const auto& sender: senders) {
    auto r = sender(src, this);
    if (r.success) {
      return r;
    }
  }

  // Reaching here most likely means
  // a bug in our program. An ill-formed
  // message from the nodejs side is a bug.
  // Now record a compound message to explain
  // what went wrong.
  Result r;
  r.success = false;
  r.explanation =
      "Failed to parse message for"
      " any of these reasons:\n";
  for (const auto& sender: senders) {
    auto r0 = sender(src, this);
    CHECK(!r0.success);
    r.explanation += " * " + r0.explanation + "\n";
  }
  return r;
}

Nmea2000Source::Result
  Nmea2000Source::send(const std::vector<std::map<std::string,
                       TaggedValue>>& src) {
  for (size_t i = 0; i < src.size(); i++) {
    Result result = send(src[i]);
    if (!result.success) {
      std::stringstream ss;
      ss << " (at msg " << i+1 << "/" << src.size() << ")";
      result.explanation += ss.str();
      return result;
    }
  }

  Result r;
  r.success = true;
  return r;
}


}  // namespace sail

