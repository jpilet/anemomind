/*
 *  Created on: 2014-05-21
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GeographicReference.h"
#include <server/common/invalidate.h>
#include <server/nautical/WGS84.h>

namespace sail {

GeographicReference::GeographicReference() {
  InvalidateScalar(&_dlon);
  InvalidateScalar(&_dlat);
}



GeographicReference::GeographicReference(const GeographicPosition<double> &pos) : _pos(pos) {
  double xyz3[3];
  WGS84<double>::toXYZLocal(pos.lon().radians(), pos.lat().radians(), pos.alt().meters(),
      xyz3, &_dlon, &_dlat);
}

GeographicReference::ProjectedPosition
  GeographicReference::map(const GeographicPosition<double> &src) const {
    return ProjectedPosition{mapXLon(src.lon()), mapYLat(src.lat())};
}

GeographicPosition<double> GeographicReference::unmap(const ProjectedPosition &src) const {
  return GeographicPosition<double>(unmapXLon(src[0]),
      unmapYLat(src[1]),
      Length<double>::meters(0));
}


Length<double> GeographicReference::mapXLon(Angle<double> lon) const {
  return Length<double>::meters(_dlon*lon.directionDifference(_pos.lon()).radians());
}

Length<double> GeographicReference::mapYLat(Angle<double> lat) const {
  return Length<double>::meters(_dlat*lat.directionDifference(_pos.lat()).radians());
}

Angle<double> GeographicReference::unmapXLon(Length<double> x) const {
  return Angle<double>::radians(x.meters()/_dlon) + _pos.lon();
}

Angle<double> GeographicReference::unmapYLat(Length<double> y) const {
  return Angle<double>::radians(y.meters()/_dlat) + _pos.lat();
}

Length<double> GeographicReference::unmapZAlt(Length<double> z) const {
  return z + _pos.alt();
}

Length<double> GeographicReference::mapZAlt(Length<double> alt) const {
  return alt - _pos.alt();
}




} /* namespace sail */
