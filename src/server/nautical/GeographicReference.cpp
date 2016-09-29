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



GeographicReference::GeographicReference(const GeographicPosition<double> &pos_) : _pos(pos_) {
  double xyz3[3] = {NAN, NAN, NAN};
  double dlon = NAN;
  double dlat = NAN;
  WGS84<double>::toXYZLocal(
      pos_.lon().radians(),
      pos_.lat().radians(),
      pos_.alt().meters(),
      xyz3, &dlon, &dlat);
  auto unit = (1.0_m/1.0_rad);
  _dlon = dlon*unit;
  _dlat = dlat*unit;
}

GeographicReference::ProjectedPosition
  GeographicReference::map(const GeographicPosition<double> &src) const {
    return ProjectedPosition{mapXLon(src.lon()), mapYLat(src.lat())};
}

GeographicPosition<double> GeographicReference::unmap(const ProjectedPosition &src) const {
  return GeographicPosition<double>(
      unmapXLon(src[0]),
      unmapYLat(src[1]),
      Length<double>::meters(0));
}

bool GeographicReference::operator== (const GeographicReference &other) const {
  return _pos == other._pos &&
      _dlat == other._dlat &&
      _dlon == other._dlon;
}


Length<double> GeographicReference::mapXLon(Angle<double> lon) const {
  return _dlon*lon.directionDifference(_pos.lon());
}

Length<double> GeographicReference::mapYLat(Angle<double> lat) const {
  return _dlat*lat.directionDifference(_pos.lat());
}

Angle<double> GeographicReference::unmapXLon(Length<double> x) const {
  return x/_dlon + _pos.lon();
}

Angle<double> GeographicReference::unmapYLat(Length<double> y) const {
  return y/_dlat + _pos.lat();
}

Length<double> GeographicReference::unmapZAlt(Length<double> z) const {
  return z + _pos.alt();
}

Length<double> GeographicReference::mapZAlt(Length<double> alt) const {
  return alt - _pos.alt();
}




} /* namespace sail */
