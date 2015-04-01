#include <device/anemobox/Nmea0183Source.h>

#include <iostream>

#include <fcntl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>

namespace sail {

class PrintAngleListenener : public Listener<Angle<double>> {
 public:
  PrintAngleListenener(std::string prefix) : _prefix(prefix) { }
  virtual void onNewValue(const ValueDispatcher<Angle<double>> &dispatcher) {
    std::cout << _prefix << ": " << dispatcher.lastValue().degrees() << std::endl;
  }
 private:
  std::string _prefix;
};

bool run(const char *filename) {
  Dispatcher dispatcher;
  Nmea0183Source nmea0183(&dispatcher);
  PrintAngleListenener twaPrint("TWA");
  dispatcher.twa()->subscribe(&twaPrint);

  if (!nmea0183.open(filename)) {
    perror(filename);
    return false;
  }

  struct pollfd pfds[1];
  pfds[0].fd = nmea0183.fd();
  pfds[0].events = POLLIN;

  while (1) {
    int r = poll(pfds, sizeof(pfds) / sizeof(pfds[0]), -1);
    if (r >= 0) {
      nmea0183.poll();
    } else {
      if (errno != EAGAIN) {
        perror("poll");
        break;
      }
    }
  }
  return true;
}

}  // namespace sail

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <path>" << std::endl;
    return -1;
  }
  sail::run(argv[1]);
  return 0;
}
