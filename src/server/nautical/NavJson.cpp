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

Poco::JSON::Object::Ptr encode(const Nav &nav) {
  Poco::JSON::Object::Ptr x(new Poco::JSON::Object());
  writeField(x, "time", nav.time());
  writeField(x, "lon", nav.geographicPosition().lon());
  writeField(x, "lat", nav.geographicPosition().lat());
  writeField(x, "alt", nav.geographicPosition().alt());
  writeField(x, "maghdg", nav.magHdg());
  writeField(x, "watspeed", nav.watSpeed());
  writeField(x, "gpsspeed", nav.gpsSpeed());
  writeField(x, "gpsbearing", nav.gpsSpeed());
  writeField(x, "aws", nav.aws());
  writeField(x, "awa", nav.awa());
  return x;
}

void decode(Poco::JSON::Object::Ptr x, Nav *out) {
  TimeStamp time;
  Angle<double> lon, lat, maghdg, gpsb, awa;
  Length<double> alt;
  Velocity<double> gpss, wats, aws;

  readField(x, "time", &time, false);
  readField(x, "lon", &lon, false);
  readField(x, "lat", &lat, false);
  readField(x, "awa", &awa, false);
  readField(x, "aws", &aws, false);
  readField(x, "alt", &alt, false);
  readField(x, "maghdg", &maghdg, false);
  readField(x, "watspeed", &wats, false);
  readField(x, "gpsspeed", &gpss, false);
  readField(x, "gpsbearing", &gpsb, false);

  *out = Nav();
  out->setTime(time);
  out->setGeographicPosition(GeographicPosition<double>(lon, lat, alt));
  out->setAwa(awa);
  out->setAws(aws);
  out->setGpsSpeed(gpss);
  out->setGpsBearing(gpsb);
  out->setMagHdg(maghdg);
  out->setWatSpeed(wats);
}

Poco::JSON::Array encode(Array<Nav> navs) { // Perhaps write a template encodeArray<T> with T = Nav in this case...
  Poco::JSON::Array arr;
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    arr.add(encode(navs[i]));
  }
  return arr;
}

void decode(Poco::JSON::Array src, Array<Nav> *dst) {
  int count = src.size();
  *dst = Array<Nav>(count);
  for (int i = 0; i < count; i++) {
    decode(src.getObject(i), dst->ptr(i));
  }
}

}
} /* namespace sail */
