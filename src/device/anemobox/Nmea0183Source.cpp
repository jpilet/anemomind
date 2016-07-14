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
    DispatcherAdaptor(const std::string &sourceName,
        Dispatcher *d) : _sourceName(sourceName), _dispatcher(d) {}

    template <DataCode Code>
    void add(const NmeaParser &parser,
        const typename TypeForCode<Code>::type &value) {
      _dispatcher->publishValue(Code, _sourceName, value);
    }

   private:
    std::string _sourceName;
    Dispatcher *_dispatcher;
  };

}

void Nmea0183Source::process(const unsigned char* buffer, int length) {
  DispatcherAdaptor adaptor(_sourceName, _dispatcher);
  for (ssize_t i = 0; i < length; ++i) {
    Nmea0183ProcessByte<DispatcherAdaptor>(
        buffer[i], &_parser, &adaptor);
  }
} 

}  // namespace sail
