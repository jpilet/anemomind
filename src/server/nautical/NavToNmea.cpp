#include <server/nautical/NavToNmea.h>

#include <boost/algorithm/string/join.hpp>
#include <server/common/string.h>

namespace sail {

double minutes(const Angle<double>& a) {
  double degrees = fabs(a.degrees());
  return (degrees - floor(degrees)) * 60;
}

std::string assembleNmeaSentence(const std::vector<std::string>& args) {
  string result = boost::algorithm::join(args, ",");

  char checksum = 0;
  for (char c : result) {
    checksum ^= c;
  }
  return "$" + result + stringFormat("*%02X", checksum);
}

std::string nmeaRmc(const Nav& nav) {
  // Example: $IIRMC,164514,A,4629.732,N,00639.787,E,03.7,212,200808,,,A*46
  GeographicPosition<double> pos = nav.geographicPosition();
  vector<string> args {
    "GPRMC",
    nav.time().toString("%H%M%S"),
    "A",
    stringFormat("%02d%07.4f", int(floor(fabs(pos.lat().degrees()))),
                 minutes(pos.lat())),
    (pos.lat() > Angle<double>::degrees(0) ? "N" : "S"),
    stringFormat("%03d%07.4f", int(floor(fabs(pos.lon().degrees()))),
                 minutes(pos.lon())),
    (pos.lon() > Angle<double>::degrees(0) ? "E" : "W"),
    stringFormat("%.1f", nav.gpsSpeed().knots()),
    stringFormat("%.1f", nav.gpsBearing().degrees()),
    nav.time().toString("%d%m%y"),
    "", "", "A" };

  return assembleNmeaSentence(args);
}

}  // namespace sail
