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

GeographicReference::ProjectedPosition
  GeographicReference::map(const GeographicPosition<double> &src) const {
  return ProjectedPosition{mapX(src.lon()), mapY(src.lat())};
}


GeographicReference::GeographicReference(const GeographicPosition<double> &pos) : _pos(pos) {
  double xyz3[3];
  WGS84<double>::toXYZLocal(pos.lon().radians(), pos.lat().radians(), pos.alt().meters(),
      xyz3, &_dlon, &_dlat);
}

void GeographicReference::mapToArray(GeographicPosition<double> src, Length<double> *xyzOut) const {
  xyzOut[0] = ;
  xyzOut[1] = Length<double>::meters(_dlat*src.lat().directionDifference(_pos.lat()).radians());
  xyzOut[2] = src.alt() - _pos.alt();
}

GeographicPosition<double> GeographicReference::unmapFromArray(Length<double> *xyzIn) const {
  Angle<double> lon = Angle<double>::radians(xyzIn[0].meters()/_dlon) + _pos.lon();
  Angle<double> lat = Angle<double>::radians(xyzIn[1].meters()/_dlat) + _pos.lat();

  return GeographicPosition<double>(lon, lat, alt);
}

Length<double> GeographicReference::mapXLon(Angle<double> lon) {
  return Length<double>::meters(_dlon*lon.directionDifference(_pos.lon()).radians());
}

Length<double> GeographicReference::mapYLat(Angle<double> lat) {
  return Length<double>::meters(_dlat*lat.directionDifference(_pos.lat()).radians());
}

Angle<double> GeographicReference::unmapXLon(Length<double> x) {
  return Angle<double>::radians(x.meters()/_dlon) + _pos.lon();
}

Angle<double> GeographicReference::unmapYLat(Length<double> y) {
  return Angle<double>::radians(y.meters()/_dlat) + _pos.lat();
}

Length<double> GeographicReferece::unmapZAlt(Length<double> z) {
  return z + _pos.alt();
}



} /* namespace sail */
