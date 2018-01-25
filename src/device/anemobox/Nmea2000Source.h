#ifndef ANEMOBOX_NMEA2000_SOURCE_H
#define ANEMOBOX_NMEA2000_SOURCE_H

#include <string>

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/n2k/PgnClasses.h>
#include <device/anemobox/n2k/CanPacket.h>

namespace sail {

class Nmea2000Source : public PgnClasses::PgnVisitor {
 public:
  Nmea2000Source(Dispatcher *dispatcher)
    : _dispatcher(dispatcher) { }

  void process(const std::string& srcName,
               int pgn,
               const unsigned char* buffer,
               int length,
               int srcAddr);

	std::vector<std::vector<uint8_t>> exportPackets(int pgn) const;
 protected:
  // TODO: Override the exportPacket methods here for the different
  // packet types.

  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::VesselHeading& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::Speed& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::GnssPositionData& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::WindData& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::PositionRapidUpdate& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::CogSogRapidUpdate& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::TimeDate& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::SystemTime& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::DirectionData& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::Rudder& packet) override;
 private:
  Dispatcher *_dispatcher;
};

}  // namespace

#endif  // ANEMOBOX_NMEA2000_SOURCE_H
