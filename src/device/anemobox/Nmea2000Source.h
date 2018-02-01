#ifndef ANEMOBOX_NMEA2000_SOURCE_H
#define ANEMOBOX_NMEA2000_SOURCE_H

#include <string>
#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/n2k/PgnClasses.h>
#include <NMEA2000.h>
#include <N2kDeviceList.h>

namespace sail {

std::string deviceNameToString(const Optional<uint64_t>& dn);

class Nmea2000Source : public PgnClasses::PgnVisitor {
 public:
  Nmea2000Source(
      tNMEA2000* source,
      Dispatcher *dispatcher);
  void process(
      const tN2kMsg& msg);

  Optional<uint64_t> getSourceName(uint8_t shortName);
 protected:
  bool apply(const tN2kMsg &c, const PgnClasses::VesselHeading& packet) override;
  bool apply(const tN2kMsg &c, const PgnClasses::Speed& packet) override;
  bool apply(const tN2kMsg &c, const PgnClasses::GnssPositionData& packet) override;
  bool apply(const tN2kMsg &c, const PgnClasses::WindData& packet) override;
  bool apply(const tN2kMsg &c, const PgnClasses::PositionRapidUpdate& packet) override;
  bool apply(const tN2kMsg &c, const PgnClasses::CogSogRapidUpdate& packet) override;
  bool apply(const tN2kMsg &c, const PgnClasses::TimeDate& packet) override;
  bool apply(const tN2kMsg &c, const PgnClasses::SystemTime& packet) override;
  bool apply(const tN2kMsg &c, const PgnClasses::DirectionData& packet) override;
  bool apply(const tN2kMsg &c, const PgnClasses::Rudder& packet) override;
 private:
  std::unique_ptr<tN2kDeviceList> _deviceList;
  std::string _lastSourceName;
  Dispatcher *_dispatcher;
};

}  // namespace

#endif  // ANEMOBOX_NMEA2000_SOURCE_H
