/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Conversion from/to Json for the Nav datatype.
 */

#include "NavJson.h"

namespace sail {

/*
 * A class that inherits from NavJasonInterface
 * specifies how a Nav is represented. The idea
 * is to represent a Nav as a Json array of numbers
 * (returned by the method 'toJson')
 * and provide a seperate array that specifies the
 * meaning of these numbers ('makeFormatSpec'). The reason
 * for this separation is to save bandwidth when transmitting many
 * Json records.
 *
 * In addition, the method getFormatVersion returns
 * an integer to uniquely identify the Json format,
 * and this integer should correspond to the first element
 * of the array returned by toJson.
 */
class NavJsonInterface {
 public:
  Poco::JSON::Object::Ptr toJsonObject(Array<Nav> navs);
  virtual ~NavJsonInterface() {}
 protected:

  // Should return an integer that is unique to the format being used.
  virtual int getFormatVersion() = 0;

  // Should return an array of strings describing the corresponding
  // entries in the array returned by toJson
  virtual Poco::JSON::Array makeFormatSpec() = 0;

  // Should return an array of values representing the Nav
  virtual Poco::JSON::Array toJson(const Nav &n) = 0;

 private:
  Poco::JSON::Array toJsonDataArray(Array<Nav> navs, int expectedLen);
};






Poco::JSON::Object::Ptr NavJsonInterface::toJsonObject(Array<Nav> navs) {
  Poco::JSON::Object::Ptr dst(new Poco::JSON::Object());
  Poco::JSON::Array format = makeFormatSpec();

  // Require any format to provide every record with a version number.
  // This is useful, for instance, if we would mix different records
  assert(format.getElement<std::string>(0) == "format-version");

  dst->set("format", format);
  int expectedLen = format.size();
  dst->set("data", toJsonDataArray(navs, expectedLen));
  return dst;
}

Poco::JSON::Array NavJsonInterface::toJsonDataArray(Array<Nav> navs, int expectedLen) {
  Poco::JSON::Array dst;
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    Poco::JSON::Array arr = toJson(navs[i]);

    assert(arr.getElement<int>(0) == getFormatVersion());

    assert(arr.size() == expectedLen);
    dst.add(arr);
  }
  return dst;
}

class NavJsonVersion001 : public NavJsonInterface {
 public:
  int getFormatVersion() {return 1;}
  Poco::JSON::Array makeFormatSpec();
  Poco::JSON::Array toJson(const Nav &n);
};

Poco::JSON::Array NavJsonVersion001::makeFormatSpec() {
  Poco::JSON::Array arr;
  arr.add("format-version");
  arr.add("time-since-1970-seconds");
  arr.add("pos-longitude-radians");
  arr.add("pos-latitude-radians");
  arr.add("pos-altitude-meters");
  arr.add("awa-radians");
  arr.add("aws-meters-per-second");
  arr.add("maghdg-radians");
  arr.add("wat-speed-meters-per-second");
  arr.add("gps-speed-meters-per-second");
  arr.add("gps-bearing-radians");
  return arr;
}

Poco::JSON::Array NavJsonVersion001::toJson(const Nav &nav) {
  Poco::JSON::Array dst;
  dst.add(getFormatVersion()); // Version of this format.
  dst.add(nav.time().seconds());
  GeographicPosition<double> pos = nav.geographicPosition();
  dst.add(pos.lon().radians());
  dst.add(pos.lat().radians());
  dst.add(pos.alt().meters());
  dst.add(nav.awa().radians());
  dst.add(nav.aws().metersPerSecond());
  dst.add(nav.magHdg().radians());
  dst.add(nav.watSpeed().metersPerSecond());
  dst.add(nav.gpsSpeed().metersPerSecond());
  dst.add(nav.gpsBearing().radians());
  return dst;
}

Poco::JSON::Object::Ptr convertToJson(Array<Nav> navs) {
  NavJsonVersion001 x;
  return x.toJsonObject(navs);
}



} /* namespace sail */
