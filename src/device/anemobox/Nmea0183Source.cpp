#include <device/anemobox/Nmea0183Source.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <device/anemobox/Nmea0183Adaptor.h>

namespace sail {

namespace {

  class DispatcherAdaptor {
   public:
    DispatcherAdaptor(Dispatcher *d) : _dispatcher(d) {}

    template <DataCode Code>
    void add(const std::string &sourceName, const typename TypeForCode<Code>::type &value) {
      _dispatcher->publishValue(Code, sourceName, value);
    }

    void setTimeOfDay(int, int, int) {}
   private:
    Dispatcher *_dispatcher;
  };

}

void Nmea0183Source::process(const unsigned char* buffer, int length) {
  DispatcherAdaptor adaptor(_dispatcher);
  for (ssize_t i = 0; i < length; ++i) {
    Nmea0183ProcessByte<DispatcherAdaptor>(_sourceName, buffer[i],
        this, &adaptor);
  }
} 

void Nmea0183Source::onRSA(const char *senderAndSentence,
                     Optional<sail::Angle<>> rudderAngle0,
                     Optional<sail::Angle<>> rudderAngle1) {
  if (rudderAngle0.defined()) {
    _dispatcher->publishValue(RUDDER_ANGLE, _sourceName, rudderAngle0.get());
  }
}

void Nmea0183Source::onXDRPitch(const char *senderAndSentence,
                                bool valid,
                                sail::Angle<double> angle) {
  if (valid) {
    _dispatcher->publishValue(PITCH, _sourceName, angle);
  }
}
void Nmea0183Source::onXDRRoll(const char *senderAndSentence,
                               bool valid,
                               sail::Angle<double> angle) {
  if (valid) {
    _dispatcher->publishValue(ROLL, _sourceName, angle);
  }
}
void Nmea0183Source::onHDM(const char *senderAndSentence, Angle<> angle) {
  _dispatcher->publishValue(MAG_HEADING, _sourceName, angle);
}

}  // namespace sail
