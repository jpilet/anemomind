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
  bool apply(const PgnClasses::VesselHeading& packet);
  bool apply(const PgnClasses::Speed& packet);
  bool apply(const PgnClasses::GnssPositionData& packet);
  bool apply(const PgnClasses::WindData& packet);
  bool apply(const PgnClasses::PositionRapidUpdate& packet);
  bool apply(const PgnClasses::CogSogRapidUpdate& packet);
  bool apply(const PgnClasses::TimeDate& packet);
  bool apply(const PgnClasses::DirectionData& packet);
 private:
  Dispatcher *_dispatcher;
  std::string _currentMessageSource;
};

}  // namespace

#endif  // ANEMOBOX_NMEA2000_SOURCE_H
