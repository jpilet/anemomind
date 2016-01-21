#ifndef ANEMOBOX_NMEA2000_SOURCE_H
#define ANEMOBOX_NMEA2000_SOURCE_H

#include <string>

#include <device/anemobox/Dispatcher.h>

namespace sail {

class Nmea2000Source {
 public:
  Nmea2000Source(Dispatcher *dispatcher)
    : _dispatcher(dispatcher) { }

  void process(const std::string& srcName,
               int pgn,
               const unsigned char* buffer,
               int length);

 private:
  Dispatcher *_dispatcher;
};

}  // namespace

#endif  // ANEMOBOX_NMEA2000_SOURCE_H
