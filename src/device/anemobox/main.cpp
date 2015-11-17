#include <device/anemobox/Nmea0183Source.h>

#include <iostream>
#include <vector>
#include <memory>
#include <server/common/PhysicalQuantityIO.h>
#include <stdio.h>

namespace sail {

namespace {

void print(const AngleDispatcher &angle) {
  std::cout << stringFormat("%.1f", angle.lastValue().degrees());
}

void print(const VelocityDispatcher &velocity) {
  std::cout << stringFormat("%.2f", velocity.lastValue().knots());
}

void print(const LengthDispatcher &length) {
  std::cout << stringFormat("%.2f", length.lastValue().nauticalMiles());
}

void print(const GeoPosDispatcher &geoPos) {
  auto pos = geoPos.lastValue();
  std::cout << "GeoPos(lon = " << pos.lon() << " lat = " << pos.lat() << ")";
}

void print(const TimeStampDispatcher &timeStamp) {
  std::cout << timeStamp.lastValue().toString();
}

void print(const AbsoluteOrientationDispatcher &orientDisp) {
  auto orient = orientDisp.lastValue();
  std::cout << "Orient(heading=" << orient.heading
    << ", roll=" << orient.roll
    << ", pitch=" << orient.pitch
    << ")";
}

template <class T>
class PrintListener : public Listener<T> {
 public:
   PrintListener(std::string prefix) : _prefix(prefix) { }
  virtual void onNewValue(const ValueDispatcher<T> &dispatcher) {
    std::cout << _prefix << ": ";
    print(dispatcher);
    std::cout << std::endl;
  }
  virtual ~PrintListener() {}
 private:
   std::string _prefix;
};

class PrintUpdates : public DispatchDataVisitor {
 public:

  virtual void run(DispatchAngleData *angle) {
    std::shared_ptr<PrintListener<Angle<double>>> anglePrinter(
        new PrintListener<Angle<double>>(angle->description()));
    angle->dispatcher()->subscribe(anglePrinter.get());
    _anglePrinters.push_back(anglePrinter);
  }

  virtual void run(DispatchVelocityData *value) {
    std::shared_ptr<PrintListener<Velocity<double>>> valuePrinter(
        new PrintListener<Velocity<double>>(value->description()));
    value->dispatcher()->subscribe(valuePrinter.get());
    _velocityPrinters.push_back(valuePrinter);
  }

  virtual void run(DispatchLengthData *value) {
    std::shared_ptr<PrintListener<Length<double>>> valuePrinter(
        new PrintListener<Length<double>>(value->description()));
    value->dispatcher()->subscribe(valuePrinter.get());
    _lengthPrinters.push_back(valuePrinter);
  }

  virtual void run(DispatchGeoPosData *value) {
    std::shared_ptr<PrintListener<GeographicPosition<double>>> valuePrinter(
        new PrintListener<GeographicPosition<double>>(value->description()));
    value->dispatcher()->subscribe(valuePrinter.get());
    _geoPosPrinters.push_back(valuePrinter);
  }

  virtual void run(DispatchTimeStampData *value) {
    std::shared_ptr<PrintListener<TimeStamp>> valuePrinter(
        new PrintListener<TimeStamp>(value->description()));
    value->dispatcher()->subscribe(valuePrinter.get());
    _timeStampPrinters.push_back(valuePrinter);
  }

  virtual void run(DispatchAbsoluteOrientationData *value) {
    std::shared_ptr<PrintListener<AbsoluteOrientation>> valuePrinter(
        new PrintListener<AbsoluteOrientation>(value->description()));
    value->dispatcher()->subscribe(valuePrinter.get());
    _orientPrinters.push_back(valuePrinter);
  }

 private:
  std::vector<std::shared_ptr<PrintListener<Angle<double>>>> _anglePrinters;
  std::vector<std::shared_ptr<PrintListener<Velocity<double>>>> _velocityPrinters;
  std::vector<std::shared_ptr<PrintListener<Length<double>>>> _lengthPrinters;
  std::vector<std::shared_ptr<PrintListener<GeographicPosition<double>>>> _geoPosPrinters;
  std::vector<std::shared_ptr<PrintListener<TimeStamp>>> _timeStampPrinters;
  std::vector<std::shared_ptr<PrintListener<AbsoluteOrientation>>> _orientPrinters;
};

}  // namespace

bool run(const char *filename) {
  Dispatcher dispatcher;
  Nmea0183Source nmea0183(&dispatcher, std::string(filename));
  PrintUpdates visitor;
  
  FILE *f = fopen(filename, "r");
  if (!f) {
    return false;
  }

  for (auto it : dispatcher.dispatchers()) {
    it.second.get()->visit(&visitor);
  }

  unsigned char buffer[128];

  while (1) {
   int len = fread(buffer, 1, 128, f);
   if (len <= 0) {
     break;
   }

   nmea0183.process(buffer, len);
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
