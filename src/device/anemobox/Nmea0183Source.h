#ifndef ANEMOBOX_NMEA0183_SOURCE_H
#define ANEMOBOX_NMEA0183_SOURCE_H

#include <string>

#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <device/anemobox/Dispatcher.h>

namespace sail {

class Nmea0183Source: public NmeaParser {
 public:
  Nmea0183Source(Dispatcher *dispatcher, const std::string& sourceName)
    : _dispatcher(dispatcher), _sourceName(sourceName) { }

  const std::string& sourceName() const { return _sourceName; }

  void process(const unsigned char* buffer, int length);

  NmeaParser *parser() { return this; }

 protected:
  virtual void onRSA(const char *senderAndSentence,
                     Optional<sail::Angle<>> rudderAngle0,
                     Optional<sail::Angle<>> rudderAngle1);
  virtual void onXDRPitch(const char *senderAndSentence,
                          bool valid,
                          sail::Angle<double> angle);
  virtual void onXDRRoll(const char *senderAndSentence,
                         bool valid,
                         sail::Angle<double> angle);
  virtual void onHDM(const char *senderAndSentence,
                     Angle<double> angle);
 private:
  Dispatcher *_dispatcher;
  std::string _sourceName;
};

}  // namespace

#endif  // ANEMOBOX_NMEA0183_SOURCE_H
