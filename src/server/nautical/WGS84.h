/*
 *  Created on: 2014-02-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef WGS84TOXYZ_H_
#define WGS84TOXYZ_H_

#include <server/common/math.h>

// -- If you use file with double, include <cmath> before this file is included --
// -- If you use file with adouble, include <adolc/adouble.h> before this file is included --

namespace sail {

template <typename T, bool useDegrees>
class Wgs84 {
 public:
  constexpr static double k2_PI = 6.283185307179586476925286766559005768394338798750211641949889184615;
  constexpr static double angleUnit2Radians = (useDegrees? (k2_PI/360.0) : 1.0);
  constexpr static double ECEFA = 6378137;
  constexpr static double ECEFE = 8.1819190842622e-2;


  // Maps lon lat and altitude to an XYZ position in an ECEF coordinate system.
  static void toXYZ(T lon, T lat, T altitude,
                         T *xyz3) {
    toXYZWithJ(lon, lat, altitude, xyz3, nullptr);
  }

  // Maps (lon, lat, altitude) to 3d position xyz3 and optionally
  // outputs the Jacobian matrix
  static void toXYZWithJ(T lon, T lat, T altitude,
                         T *xyz3, T *J3x3ColMajorOut) {
    T theta = lat*angleUnit2Radians;
    T phi = lon*angleUnit2Radians;
    T E2 = sqr(ECEFE);
    T Ndenom = sqrt(1 - E2*sqr(sin(theta)));
    T N = ECEFA/Ndenom;
    T cosTheta = cos(theta);
    T cosPhi = cos(phi);
    T sinPhi = sin(phi);
    T sinTheta = sin(theta);

    T Nh = N + altitude;

    xyz3[0] = Nh*cosTheta*cosPhi;
    xyz3[1] = Nh*cosTheta*sinPhi;
    T oneMinusE2 = (1 - sqr(ECEFE));
    xyz3[2] = (oneMinusE2*N + altitude)*sinTheta;

    if (J3x3ColMajorOut != nullptr) {
      T dNDTheta = ECEFA*E2*sinTheta*cosTheta/(Ndenom*sqrt(Ndenom));

      // dPhi
      T dXdPhi = -Nh*cosTheta*sinPhi;
      T dYdPhi = Nh*cosTheta*cosPhi;
      T dZdPhi = 0.0;

      // dTheta
      T dXdTheta = dNDTheta*cosTheta*cosPhi - Nh*sinTheta*cosPhi;
      T dYdTheta = dNDTheta*cosTheta*sinPhi - Nh*sinTheta*sinPhi;
      T dZdTheta = oneMinusE2*dNDTheta*sinTheta + (oneMinusE2*N + altitude)*cosTheta;

      // dH
      T dXdH = cosTheta*cosPhi;
      T dYdH = cosTheta*sinPhi;
      T dZdH = sinTheta;

      // Col 1
      J3x3ColMajorOut[0] = dXdPhi*angleUnit2Radians;
      J3x3ColMajorOut[1] = dYdPhi*angleUnit2Radians;
      J3x3ColMajorOut[2] = dZdPhi*angleUnit2Radians;

      // Col 2
      J3x3ColMajorOut[3] = dXdTheta*angleUnit2Radians;
      J3x3ColMajorOut[4] = dYdTheta*angleUnit2Radians;
      J3x3ColMajorOut[5] = dZdTheta*angleUnit2Radians;

      // Col 3
      J3x3ColMajorOut[6] = dXdH;
      J3x3ColMajorOut[7] = dYdH;
      J3x3ColMajorOut[8] = dZdH;
    }

  }

  static void posAndDirToXYZ(T lon, T lat, T altitude, T dir, T *xyzPosOut, T *xyzDirUnitVectorOut) {
    T J[9];
    toXYZWithJ(lon, lat, altitude, xyzPosOut, J);
    T *dlon = J + 0;
    T *dlat = J + 3;
    T dNorth = cos(angleUnit2Radians*dir);
    T dEast = sin(angleUnit2Radians*dir);
    T len2 = 0.0;
    for (int i = 0; i < 3; i++) {
      T elem = dNorth*dlat[i] + dEast*dlon[i];
      xyzDirUnitVectorOut[i] = elem;
      len2 += elem*elem;
    }


    T oneOverL = 1.0/sqrt(len2);
    for (int i = 0; i < 3; i++) {
      xyzDirUnitVectorOut[i] *= oneOverL;
    }
  }


  /*************************************************************
   * Code adopted from the NmeaParser library
   * The testcases illustrate that this code maps to the same position
   * as the code above. In other words, the xyz3 output of this method
   * is the same as the xyz3 output of the toXYZWithJ method.
   *
   * The main difference is that this code outputs scalars dlon1 and dlat1 whereas
   * the toXYZWithJ method outputs a full Jacobian matrix.
   */
  constexpr static double a = 6378137; // semi-major axis of ellipsoid
  constexpr static double f = 1.0/298.257223563; // flatening of ellipsoid
  // Maps (lon, lat, altitude) to a 3D position xyz3.
  // Optionally outputs two scalars, dlon1 and dlat1, that are the derivatives of
  // the norm of the xyz position w.r.t. lon and lat.
  static void toXYZCopiedFromNmeaParserLib(T lon, T lat, T altitude,
                         T *xyz3, T *dlon1, T *dlat1) {
    T latRad = lat * angleUnit2Radians;
    T lonRad = lon * angleUnit2Radians;
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

    if (dlon1) {
      *dlon1 = sqrt(t13*t13 + t26*t26)*angleUnit2Radians;
    }

    if (dlat1) {
      T u = (t37*coslon-t23*coslon);
      T v = (t37*sinlon-t23*sinlon);
      T w = (t16*t31*t4*coslat+t36*coslat);
      *dlat1 = sqrt(u*u + v*v + w*w)*angleUnit2Radians;
    }

    if (xyz3) {
      xyz3[0] = t26;
      xyz3[1] = t13;
      xyz3[2] = t36*sinlat;
    }
  }
};



}



#endif /* WGS84TOXYZ_H_ */
