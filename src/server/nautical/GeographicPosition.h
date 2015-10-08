/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GEOGRAPHICPOSITION_H_
#define GEOGRAPHICPOSITION_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

template <typename T>
class GeographicPosition {
public:
  typedef GeographicPosition<T> ThisType;

  GeographicPosition() {}
  GeographicPosition(Angle<T> longitude, Angle<T> latitude, Length<T> altitude = Length<T>::meters(0)) :
    _lon(longitude),
    _lat(latitude),
    _alt(altitude) {}

  Angle<T> lon() const {return _lon;}
  Angle<T> lat() const {return _lat;}
  Length<T> alt() const {return _alt;}

  void setLon(Angle<T> lon) {_lon = lon;}
  void setLat(Angle<T> lat) {_lat = lat;}

  template <typename DstType>
  operator GeographicPosition<DstType>() const {
    return GeographicPosition<DstType>(
        Angle<DstType>::radians(_lon.radians()),
        Angle<DstType>::radians(_lat.radians()),
        Length<DstType>::meters(_alt.meters()));
  }

  bool operator== (const ThisType &other) const {
    return _lon.eqWithNan(other._lon) &&
        _lat.eqWithNan(other._lat) &&
        _alt.eqWithNan(other._alt);
  }
private:
  Angle<T> _lon, _lat;
  Length<T> _alt; // E.g. 300 metres if we are sailing on Lac Léman
};

} /* namespace sail */

#endif /* GEOGRAPHICPOSITION_H_ */
