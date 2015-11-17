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

#include <server/common/Json.impl.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const Nav &nav) {
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
  serializeField(x, "externalTws", nav.externalTws());
  serializeField(x, "externalTwa", nav.externalTwa());
  serializeField(x, "twdir", nav.trueWindOverGround().angle());
  serializeField(x, "tws", nav.trueWindOverGround().norm());
  serializeField(x, "id", nav.id());
  serializeField(x, "boat-id", nav.boatId());
  return Poco::Dynamic::Var(x);
}

bool deserialize(Poco::Dynamic::Var x, Nav *out) {
  try {
    TimeStamp time;
    Angle<double> lon, lat, maghdg, gpsb, awa, externalTwa;
    Length<double> alt;
    Velocity<double> gpss, wats, aws, externalTws;

    std::string id, boatId;

    // Some fields may be missing. Therefore,
    // don't return false if deserializeField returns false.
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
    deserializeField(x, "externalTws", &externalTws);
    deserializeField(x, "externalTwa", &externalTwa);
    Angle<double> twdir;
    Velocity<double> tws;

    *out = Nav();

    if (deserializeField(x, "twdir", &twdir) && deserializeField(x, "tws", &tws)) {
      out->setTrueWindOverGround(HorizontalMotion<double>::polar(tws, twdir));
    }

    out->setTime(time);
    out->setGeographicPosition(GeographicPosition<double>(lon, lat, alt));
    out->setAwa(awa);
    out->setAws(aws);
    out->setGpsSpeed(gpss);
    out->setGpsBearing(gpsb);
    out->setMagHdg(maghdg);
    out->setWatSpeed(wats);
    out->setBoatId(boatId);
    out->setExternalTwa(externalTwa);
    out->setExternalTws(externalTws);
    return true;
  } catch (Poco::Exception &e) {
    return false;
  }
}

}  // namespace json
}  // namespace sail
