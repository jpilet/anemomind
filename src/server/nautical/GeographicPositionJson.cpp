/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/PhysicalQuantityJson.h>
#include "GeographicPosition.h"
#include <server/nautical/GeographicPosition.h>
#include <server/common/JsonObjDeserializer.h>
#include "GeographicPositionJson.h"

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const GeoPosd &posd) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("lon", serialize(posd.lon()));
  obj->set("lat", serialize(posd.lat()));
  obj->set("alt", serialize(posd.alt()));
  return Poco::Dynamic::Var(obj);
}

bool deserialize(Poco::Dynamic::Var src, GeoPosd *dst) {
  ObjDeserializer deser(src);
  Angle<double> lon, lat;
  Length<double> alt;
  deser.get("lon", &lon);
  deser.get("lat", &lat);
  deser.get("alt", &alt);

  if (deser.success()) {
    *dst = GeoPosd(lon, lat, alt);
    return true;
  }
  return false;
}


}
} /* namespace sail */
