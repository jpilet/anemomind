/*
 *  Created on: 2014-02-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Based on the function 'wgs84ToXYZ' in NmeaParser, but
 *  with a parameterized type T, that can be for instance double or adouble.
 */

#ifndef WGS84TOXYZ_H_
#define WGS84TOXYZ_H_

// -- If you use file with double, include <cmath> before this file is included --
// -- If you use file with adouble, include <adolc/adouble.h> before this file is included --

namespace sail {

template <typename T>
void wgs84ToXYZGeneric(T lonDeg, T latDeg, T altitude,
                       T *xyz, T *dlon, T *dlat) {
  const double k2_PI = 6.283185307179586476925286766559005768394338798750211641949889184615;
  const double kDeg2Grad = (k2_PI/360.0);
  const double a = 6378137; // semi-major axis of ellipsoid
  const double f = 1.0/298.257223563; // flatening of ellipsoid
  T latRad = latDeg * kDeg2Grad;
  T lonRad = lonDeg * kDeg2Grad;
  T sinlat = sin(latRad);
  T coslat = cos(latRad);
  T sinlon = sin(lonRad);
  T coslon = cos(lonRad);
  T e2 =  f*(2-f); //  eccentricity^2
  T t3,t4,t5,t6,t8,t9,t11,t13,t16,t17,t18,t19,t23,t26,t31,t36,t37;

  t3 = sinlat*sinlat;
  t4 = e2*t3;
  t5 = 1.0-t4;
  t6 = sqrt(t5);
  t8 = a/t6;
  t9 = t8+altitude;
  t11 = t9*coslat;
  t13 = t11*sinlon;
  t16 = a/t6/t5;
  t17 = t16*e2;
  t18 = coslat*coslat;
  t19 = sinlat*t18;
  t23 = t9*sinlat;
  t26 = t11*coslon;
  t31 = 1.0-e2;
  t36 = t8*t31+altitude;
  t37 = t17*t19;

  if (dlon) {
    *dlon = sqrt(t13*t13 + t26*t26)*kDeg2Grad;
  }

  if (dlat) {
    T u = (t37*coslon-t23*coslon);
    T v = (t37*sinlon-t23*sinlon);
    T w = (t16*t31*t4*coslat+t36*coslat);
    *dlat = sqrt(u*u + v*v + w*w)*kDeg2Grad;
  }

  if (xyz) {
    xyz[0] = t26;
    xyz[1] = t13;
    xyz[2] = t36*sinlat;
  }
}


}



#endif /* WGS84TOXYZ_H_ */
