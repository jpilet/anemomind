/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Conversion from/to Json for the Nav datatype.
 */

#include "NavJson.h"

#include <server/common/PhysicalQuantityJson.h>
#include <server/common/TimeStampJson.h>
#include <server/common/Json.h>

namespace sail {
namespace json {

CommonJson::Ptr serialize(const Nav &nav) {
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
  serializeField(x, "id", nav.id());
  serializeField(x, "boat-id", nav.boatId());
  return CommonJson::Ptr(new CommonJsonObject(x));
}

void deserialize(CommonJson::Ptr x, Nav *out) {
  TimeStamp time;
  Angle<double> lon, lat, maghdg, gpsb, awa;
  Length<double> alt;
  Velocity<double> gpss, wats, aws;

  std::string id, boatId;

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
  deserializeField(x, "boat-id", &boatId);

  *out = Nav();
  out->setTime(time);
  out->setGeographicPosition(GeographicPosition<double>(lon, lat, alt));
  out->setAwa(awa);
  out->setAws(aws);
  out->setGpsSpeed(gpss);
  out->setGpsBearing(gpsb);
  out->setMagHdg(maghdg);
  out->setWatSpeed(wats);
  out->setBoatId(boatId);
}


}
} /* namespace sail */
