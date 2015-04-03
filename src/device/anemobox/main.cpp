#include <device/anemobox/Nmea0183Source.h>

#include <iostream>
#include <vector>
#include <memory>

#include <fcntl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>

namespace sail {

namespace {

void print(const AngleDispatcher &angle) {
  std::cout << stringFormat("%.1f", angle.lastValue().degrees());
}

void print(const VelocityDispatcher &velocity) {
  std::cout << stringFormat("%.2f", velocity.lastValue().knots());
}

template <class T>
class PrintListenener : public Listener<T> {
 public:
   PrintListenener(std::string prefix) : _prefix(prefix) { }
  virtual void onNewValue(const ValueDispatcher<T> &dispatcher) {
    std::cout << _prefix << ": ";
    print(dispatcher);
    std::cout << std::endl;
  }
 private:
   std::string _prefix;
};

class PrintUpdates : public DispatchDataVisitor {
 public:
  virtual void run(DispatchAngleData *angle) {
    std::shared_ptr<PrintListenener<Angle<double>>> anglePrinter(
        new PrintListenener<Angle<double>>(angle->description()));
    angle->dispatcher()->subscribe(anglePrinter.get());
    _anglePrinters.push_back(anglePrinter);
  }

  virtual void run(DispatchVelocityData *value) {
    std::shared_ptr<PrintListenener<Velocity<double>>> valuePrinter(
        new PrintListenener<Velocity<double>>(value->description()));
    value->dispatcher()->subscribe(valuePrinter.get());
    _velocityPrinters.push_back(valuePrinter);
  }
 private:
  std::vector<std::shared_ptr<PrintListenener<Angle<double>>>> _anglePrinters;
  std::vector<std::shared_ptr<PrintListenener<Velocity<double>>>> _velocityPrinters;
};

}  // namespace

bool run(const char *filename) {
  Dispatcher dispatcher;
  Nmea0183Source nmea0183(&dispatcher);
  PrintUpdates visitor;

  for (auto value : dispatcher.data()) {
    value.second->visit(&visitor);
  }

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
