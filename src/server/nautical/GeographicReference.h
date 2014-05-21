/*
 *  Created on: 2014-05-21
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Based on the GeoRef class in the NmeaParser library,
 *  but using GeographicPosition
 */

#ifndef GEOGRAPHICREFERENCE_H_
#define GEOGRAPHICREFERENCE_H_

namespace sail {

class GeographicReference {
 public:
  GeographicReference();
  GeographicReference(const GeographicPosition<double> &pos);
  void map(GeographicPosition<double> src, Length<double> *xyzOut);
  void unmap(Length<double> *xyzIn, GeographicPosition<double> *posOut);
 private:
  double _dlon, _dlat;
  GeographicPosition<double> _pos;
};

} /* namespace sail */

#endif /* GEOGRAPHICREFERENCE_H_ */
