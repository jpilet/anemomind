/*
 *  Created on: 2014-03-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Miscellaneous calculations on geographic positions
 */

#ifndef GEOCALC_H_
#define GEOCALC_H_

#include <server/common/Array.h>
#include <server/nautical/GeographicPosition.h>

namespace sail {

// Infers a geographic position from 3D ECEF coordinates
GeographicPosition<double> toGeographicPosition(Length<double> *XYZ);

// Computes a mean geographic position. Simply computing the mean of
// latitudes and longitudes in terms of angles may be unstable due to periodicity.
GeographicPosition<double> mean(Array<GeographicPosition<double> > positions);


// Class based on the GeoRef class in <device/Arduino/libraries/NmeaParser/NmeaParser.h>
// but making use of the new template class and with support for automatic differentiation.
//
// The reference position can be computed as the 'mean' of several positions
class GeographicReference {
 public:
  GeographicReference(GeographicPosition<double> refpos);

 private:
  GeographicPosition<double> _refpos;
  double _dlon, _dlat;
};

} /* namespace sail */

#endif /* GEOCALC_H_ */
