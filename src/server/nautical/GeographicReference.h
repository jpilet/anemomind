/*
 *  Created on: 2014-05-21
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Based on the GeoRef class in the NmeaParser library,
 *  but using GeographicPosition
 */

#ifndef GEOGRAPHICREFERENCE_H_
#define GEOGRAPHICREFERENCE_H_

#include <server/nautical/GeographicPosition.h>

namespace sail {

class GeographicReference {
 public:
  GeographicReference();
  GeographicReference(const GeographicPosition<double> &pos);

  typedef Vectorize<Length<double>, 2> ProjectedPosition;

  ProjectedPosition map(const GeographicPosition<double> &src) const;

  void mapToArray(GeographicPosition<double> src, Length<double> *xyzOut) const;
  GeographicPosition<double> unmapFromArray(Length<double> *xyzIn) const;
 private:
  Length<double> mapXLon(Angle<double> lon);
  Length<double> mapYLat(Angle<double> lat);
  Angle<double> unmapXLon(Length<double> x);
  Angle<double> unmapYLat(Length<double> y);
  Length<double> unmapZAlt(Length<double> z);

  double _dlon, _dlat;
  GeographicPosition<double> _pos;
};

} /* namespace sail */

#endif /* GEOGRAPHICREFERENCE_H_ */
