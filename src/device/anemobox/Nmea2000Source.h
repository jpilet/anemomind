#ifndef ANEMOBOX_NMEA2000_SOURCE_H
#define ANEMOBOX_NMEA2000_SOURCE_H

#include <string>
#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/n2k/PgnClasses.h>
#include <NMEA2000.h>
#include <N2kDeviceList.h>

namespace sail {

std::string deviceNameToString(const Optional<uint64_t>& dn);

class Nmea2000Source :
    public PgnClasses::PgnVisitor,
    public tNMEA2000::tMsgHandler {
 public:
  Nmea2000Source(
      tNMEA2000* source,
      Dispatcher *dispatcher);

  void HandleMsg(const tN2kMsg &N2kMsg) override;

  Optional<uint64_t> getSourceName(uint8_t shortName);


  struct SendOptions {
    SendOptions();
    int priority;
    uint8_t destination;
  };

  // Returns true iff the tNMEA2000 class accepts to send it.
  bool send(
      int sourceDeviceIndex,
      const PgnClasses::PgnBaseClass& msg,
      const SendOptions& opts = SendOptions());
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
  bool apply(const tN2kMsg &c, const PgnClasses::Attitude& packet) override;
  bool apply(const tN2kMsg &c, const PgnClasses::RateOfTurn& packet) override;
  bool apply(const tN2kMsg &c,
             const PgnClasses::EngineParametersRapidUpdate& packet) override; 
 private:
  std::unique_ptr<tN2kDeviceList> _deviceList;
  std::string _lastSourceName;
  Dispatcher *_dispatcher;
};

}  // namespace

#endif  // ANEMOBOX_NMEA2000_SOURCE_H
