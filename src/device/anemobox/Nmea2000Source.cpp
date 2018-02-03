#include <device/anemobox/Nmea2000Source.h>
#include <device/anemobox/Nmea2000Utils.h>

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

const char* n2kSendResultToString(N2kSendResult r) {
  switch (r) {
  case N2kSendResult::Success: return "Success";
  case N2kSendResult::N2kError: return "NMEA2000-related error";
  case N2kSendResult::BadUnit: return "Bad unit";
  case N2kSendResult::BadMessageFormat: return "Bad message format";
  };
  CHECK(false);
  return nullptr;
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
// returns a result.
typedef N2kSendResult
    (*TaggedValueSender)(
        int, // <-- device index
        const std::map<std::string, TaggedValue>&,
        Nmea2000Source*);

N2kSendResult orError(
    const N2kSendResult& a,
        const N2kSendResult& b) {
  return std::max(a, b);
}

N2kSendResult closestToBeingSent(
    const N2kSendResult& a,
    const N2kSendResult& b) {
  return std::min(a, b);
}



N2kSendResult extractTypedValue(
    const TaggedValue& src, Angle<double>* dst) {
  if (src.tag == "deg" || src.tag == "" || src.tag == "degrees") {
    *dst = Angle<double>::degrees(src.value);
    return N2kSendResult::Success;
  } else if (src.tag == "rad" || src.tag == "radians") {
    *dst = Angle<double>::radians(src.value);
    return N2kSendResult::Success;
  }
  return N2kSendResult::BadUnit;
}

template <typename T>
N2kSendResult extract(
    const std::map<std::string, TaggedValue>& src,
    const char* key,
    T* dst) {
  auto f = src.find(key);
  return f == src.end()?
    N2kSendResult::BadMessageFormat
    : extractTypedValue(f->second, dst);
}

N2kSendResult sendPositionRapidUpdate(
    int deviceIndex,
    const std::map<std::string, TaggedValue>& src,
    Nmea2000Source* dst) {
  Angle<double> lon = 0.0_rad;
  Angle<double> lat = 0.0_rad;
  auto r = orError(
      extract(src, "longitude", &lon),
      extract(src, "latitude", &lat));
  if (r != N2kSendResult::Success) {
    return r;
  }
  PositionRapidUpdate x;
  x.longitude = lon;
  x.latitude = lat;
  if (dst->send(deviceIndex, x)) {
    return N2kSendResult::Success;
  } else {
    return N2kSendResult::N2kError;
  }
}


N2kSendResult Nmea2000Source::send(
    const std::map<std::string, TaggedValue>& src) {

  int deviceIndex = 0; // What should it be?

  /*
   * We do a linear search through all the possible senders.
   * Will it be a performance problem? Only if we have sufficiently
   * many message formats. In that case, we can always dispatch
   * based on some kind of tag.
   *
   * It could be that some sender uses a subset of the keys used
   * by another sender. In that case, we should take care to
   * test the more specific case first.
   */
  static std::vector<TaggedValueSender> senders{
    &sendPositionRapidUpdate
  };

  N2kSendResult r = N2kSendResult::BadMessageFormat;
  for (const auto& sender: senders) {

    // We track the most promising outcome...
    r = closestToBeingSent(r, sender(deviceIndex, src, this));

    // ...and if it happens to be a success, we stop.
    if (r == N2kSendResult::Success) {
      return r;
    }
  }

  // ...Otherwise, we return the error of the most promising
  // outcome.
  return r;
}

N2kSendResult Nmea2000Source::send(
    const std::vector<std::map<std::string,
    TaggedValue>>& src) {
  for (size_t i = 0; i < src.size(); i++) {
    auto result = send(src[i]);
    // Whenever we encounter an error, we stop and report that error.
    // Or would it be better to try to send all messages, and
    // report the most severe error at the end?
    //
    // Note that, if there is no bug in the code, the only
    // error we would expect at runtime is N2kSendResult::N2kError
    // The other errors are just there to help debugging from
    // the JavaScript side.
    if (result != N2kSendResult::Success) {
      return result;
    }
  }
  return N2kSendResult::Success;
}


}  // namespace sail

