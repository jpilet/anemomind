#ifndef ANEMOBOX_NMEA0183_SOURCE_H
#define ANEMOBOX_NMEA0183_SOURCE_H

#include <string>

#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <device/anemobox/Dispatcher.h>

namespace sail {

class Nmea0183Source {
 public:
  Nmea0183Source(Dispatcher *dispatcher)
    : _dispatcher(dispatcher) { }

  const char *sourceName() const { return "NMEA0183"; }

  void process(const unsigned char* buffer, int length);

 private:
  Dispatcher *_dispatcher;
  NmeaParser _parser;
};

}  // namespace

#endif  // ANEMOBOX_NMEA0183_SOURCE_H
