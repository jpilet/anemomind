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
  _dispatcher->publish<RUDDER_ANGLE>(_sourceName, rudderAngle0);
}

}  // namespace sail
