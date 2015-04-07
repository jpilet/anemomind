#ifndef ANEMOBOX_NMEA0183_SOURCE_H
#define ANEMOBOX_NMEA0183_SOURCE_H

#include <string>

#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <device/anemobox/Dispatcher.h>

namespace sail {

class Nmea0183Source {
 public:
  Nmea0183Source(Dispatcher *dispatcher)
    : _dispatcher(dispatcher), _fd(-1) { }

  bool open(const char *path);

  int fd() const { return _fd; }
  void poll();

  const char *sourceName() const { return "NMEA0183"; }

 private:
  Dispatcher *_dispatcher;
  NmeaParser _parser;
  int _fd;
};

}  // namespace

#endif  // ANEMOBOX_NMEA0183_SOURCE_H
