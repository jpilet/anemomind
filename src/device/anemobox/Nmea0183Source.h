#ifndef ANEMOBOX_NMEA0183_SOURCE_H
#define ANEMOBOX_NMEA0183_SOURCE_H

#include <string>

#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <device/anemobox/Dispatcher.h>

namespace sail {

class Nmea0183Source {
 public:
  Nmea0183Source(Dispatcher *dispatcher, const std::string& sourceName)
    : _dispatcher(dispatcher), _sourceName(sourceName) { }

  const std::string& sourceName() const { return _sourceName; }

  void process(const unsigned char* buffer, int length);

  NmeaParser *parser() {return &_parser;}
 private:
  Dispatcher *_dispatcher;
  NmeaParser _parser;
  std::string _sourceName;
};

}  // namespace

#endif  // ANEMOBOX_NMEA0183_SOURCE_H
