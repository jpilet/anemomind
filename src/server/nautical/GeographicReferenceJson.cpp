/*
 *  Created on: Jun 17, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GeographicReferenceJson.h"
#include <server/nautical/GeographicReference.h>
#include <server/nautical/GeographicPositionJson.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const GeographicReference &geoRef) {
  return serialize(geoRef.pos());
}

bool deserialize(Poco::Dynamic::Var src, GeographicReference *dst) {
  GeographicPosition<double> pos;
  bool value = deserialize(src, &pos);
  *dst = GeographicReference(pos);
  return value;
}

}
} /* namespace sail */
