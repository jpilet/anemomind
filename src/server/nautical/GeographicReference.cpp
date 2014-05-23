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

void GeographicReference::map(GeographicPosition<double> src, Length<double> *xyzOut) {
  xyzOut[0] = Length<double>::meters(_dlon*src.lon().directionDifference(_pos.lon()).radians());
  xyzOut[1] = Length<double>::meters(_dlat*src.lat().directionDifference(_pos.lat()).radians());
  xyzOut[2] = src.alt() - _pos.alt();
}

GeographicPosition<double> GeographicReference::unmap(Length<double> *xyzIn) {
  Angle<double> lon = Angle<double>::radians(xyzIn[0].meters()/_dlon) + _pos.lon();
  Angle<double> lat = Angle<double>::radians(xyzIn[1].meters()/_dlat) + _pos.lat();
  Length<double> alt = xyzIn[2] + _pos.alt();
  return GeographicPosition<double>(lon, lat, alt);
}


} /* namespace sail */
