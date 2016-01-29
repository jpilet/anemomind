#ifndef ANEMOBOX_NMEA2000_SOURCE_H
#define ANEMOBOX_NMEA2000_SOURCE_H

#include <string>

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/n2k/PgnClasses.h>

namespace sail {

class Nmea2000Source : public PgnClasses::PgnVisitor {
 public:
  Nmea2000Source(Dispatcher *dispatcher)
    : _dispatcher(dispatcher) { }

  void process(const std::string& srcName,
               int pgn,
               const unsigned char* buffer,
               int length);
 protected:
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::VesselHeading& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::Speed& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::GnssPositionData& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::WindData& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::PositionRapidUpdate& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::CogSogRapidUpdate& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::TimeDate& packet) override;
  bool apply(const PgnClasses::CanPacket &c, const PgnClasses::DirectionData& packet) override;
 private:
  Dispatcher *_dispatcher;
};

}  // namespace

#endif  // ANEMOBOX_NMEA2000_SOURCE_H
