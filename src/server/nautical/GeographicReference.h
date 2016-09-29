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
  typedef Per<Length<double>, Angle<double>> DType;

  GeographicReference();
  GeographicReference(const GeographicPosition<double> &pos_);

  typedef Vectorize<Length<double>, 2> ProjectedPosition;

  ProjectedPosition map(const GeographicPosition<double> &src) const;
  GeographicPosition<double> unmap(const ProjectedPosition &src) const;

  const GeographicPosition<double> &pos() const {return _pos;}

  bool operator== (const GeographicReference &other) const;

  DType dlon() const {
    return _dlon;
  }

  DType dlat() const {
    return _dlat;
  }
 private:
  Length<double> mapXLon(Angle<double> lon) const;
  Length<double> mapYLat(Angle<double> lat) const;
  Angle<double> unmapXLon(Length<double> x) const;
  Angle<double> unmapYLat(Length<double> y) const;
  Length<double> mapZAlt(Length<double> alt) const;
  Length<double> unmapZAlt(Length<double> z) const;

  DType _dlon, _dlat;
  GeographicPosition<double> _pos;
};

} /* namespace sail */

#endif /* GEOGRAPHICREFERENCE_H_ */
