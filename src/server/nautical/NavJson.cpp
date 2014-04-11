/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Conversion from/to Json for the Nav datatype.
 */

#include "NavJson.h"

#include <server/common/PhysicalQuantityJson.h>
#include <server/common/TimeStampJson.h>

namespace sail {
namespace json {

Poco::JSON::Object::Ptr serialize(const Nav &nav) {
  Poco::JSON::Object::Ptr x(new Poco::JSON::Object());
  serializeField(x, "time", nav.time());
  serializeField(x, "lon", nav.geographicPosition().lon());
  serializeField(x, "lat", nav.geographicPosition().lat());
  serializeField(x, "alt", nav.geographicPosition().alt());
  serializeField(x, "maghdg", nav.magHdg());
  serializeField(x, "watspeed", nav.watSpeed());
  serializeField(x, "gpsspeed", nav.gpsSpeed());
  serializeField(x, "gpsbearing", nav.gpsBearing());
  serializeField(x, "aws", nav.aws());
  serializeField(x, "awa", nav.awa());
  x->set("id", nav.id());
  return x;
}

void deserialize(Poco::JSON::Object::Ptr x, Nav *out) {
  TimeStamp time;
  Angle<double> lon, lat, maghdg, gpsb, awa;
  Length<double> alt;
  Velocity<double> gpss, wats, aws;

  deserializeField(x, "time", &time);
  deserializeField(x, "lon", &lon);
  deserializeField(x, "lat", &lat);
  deserializeField(x, "awa", &awa);
  deserializeField(x, "aws", &aws);
  deserializeField(x, "alt", &alt);
  deserializeField(x, "maghdg", &maghdg);
  deserializeField(x, "watspeed", &wats);
  deserializeField(x, "gpsspeed", &gpss);
  deserializeField(x, "gpsbearing", &gpsb);

  std::string id = x->getValue<std::string>("id");

  *out = Nav();
  out->setTime(time);
  out->setGeographicPosition(GeographicPosition<double>(lon, lat, alt));
  out->setAwa(awa);
  out->setAws(aws);
  out->setGpsSpeed(gpss);
  out->setGpsBearing(gpsb);
  out->setMagHdg(maghdg);
  out->setWatSpeed(wats);
  out->setId(id);
}

Poco::JSON::Array serialize(Array<Nav> navs) { // Perhaps write a template encodeArray<T> with T = Nav in this case...
  Poco::JSON::Array arr;
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    arr.add(serialize(navs[i]));
  }
  return arr;
}

void deserialize(Poco::JSON::Array src, Array<Nav> *dst) {
  int count = src.size();
  *dst = Array<Nav>(count);
  for (int i = 0; i < count; i++) {
    deserialize(src.getObject(i), dst->ptr(i));
  }
}

}
} /* namespace sail */
