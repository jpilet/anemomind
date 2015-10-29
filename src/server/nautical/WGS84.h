/*
 *  Created on: 2014-02-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef WGS84TOXYZ_H_
#define WGS84TOXYZ_H_

#include <server/common/math.h>
#include <server/nautical/GeographicPosition.h>

#include <iostream>
#include <server/common/string.h>
// Make sure functions exist for T by including <cmath> or <adolc/adouble.h>


namespace sail {

template <typename T>
class WGS84 {
 public:
  constexpr static double k2_PI = 6.283185307179586476925286766559005768394338798750211641949889184615;
  constexpr static double ECEFA = 6378137;
  constexpr static double ECEFE = 8.1819190842622e-2;


  static void toXYZ(const GeographicPosition<T> &pos, Length<T> *xyzOut) {
    T xyzMetres[3];
    toXYZ(pos.lon().radians(), pos.lat().radians(), pos.alt().meters(),
      xyzMetres);
    for (int i = 0; i < 3; i++) {
      xyzOut[i] = Length<T>::meters(xyzMetres[i]);
    }
  }

  static void posAndDirToXYZ(const GeographicPosition<T> pos, Angle<T> dir,
      Length<T> *xyz3, T *xyzDirUnitVectorOut) {
      T xyz3Metres[3];
      posAndDirToXYZ(pos.lon().radians(), pos.lat().radians(), pos.alt().meters(),
        dir.radians(), xyz3Metres, xyzDirUnitVectorOut);
      for (int i = 0; i < 3; i++) {
        xyz3[i] = Length<T>::meters(xyz3Metres[i]);
      }
  }

  // Maps (lon, lat, altitude) to 3d position xyz3 and optionally
  // outputs the Jacobian matrix
  static void toXYZWithJ(T lonRad, T latRad, T altitudeMetres,
                         T *xyz3MetresOut, T *J3x3ColMajorOut) {
    T theta = latRad;
    T phi = lonRad;
    T E2 = sqr(ECEFE);
    T Ndenom = sqrt(1 - E2*sqr(sin(theta)));
    T N = ECEFA/Ndenom;
    T cosTheta = cos(theta);
    T cosPhi = cos(phi);
    T sinPhi = sin(phi);
    T sinTheta = sin(theta);

    T Nh = N + altitudeMetres;
    T oneMinusE2 = (1 - E2);

    xyz3MetresOut[0] = Nh*cosTheta*cosPhi;
    xyz3MetresOut[1] = Nh*cosTheta*sinPhi;
    xyz3MetresOut[2] = (oneMinusE2*N + altitudeMetres)*sinTheta;

    if (J3x3ColMajorOut != nullptr) {
      T dNDTheta = ECEFA*E2*sinTheta*cosTheta/(Ndenom*sqrt(Ndenom));

      // dPhi
      T dXdPhi = -Nh*cosTheta*sinPhi;
      T dYdPhi = Nh*cosTheta*cosPhi;
      T dZdPhi = 0.0;

      // dTheta
      T dXdTheta = dNDTheta*cosTheta*cosPhi - Nh*sinTheta*cosPhi;
      T dYdTheta = dNDTheta*cosTheta*sinPhi - Nh*sinTheta*sinPhi;
      T dZdTheta = oneMinusE2*dNDTheta*sinTheta + (oneMinusE2*N + altitudeMetres)*cosTheta;

      // dH
      T dXdH = cosTheta*cosPhi;
      T dYdH = cosTheta*sinPhi;
      T dZdH = sinTheta;

      // Col 1
      J3x3ColMajorOut[0] = dXdPhi;
      J3x3ColMajorOut[1] = dYdPhi;
      J3x3ColMajorOut[2] = dZdPhi;

      // Col 2
      J3x3ColMajorOut[3] = dXdTheta;
      J3x3ColMajorOut[4] = dYdTheta;
      J3x3ColMajorOut[5] = dZdTheta;

      // Col 3
      J3x3ColMajorOut[6] = dXdH;
      J3x3ColMajorOut[7] = dYdH;
      J3x3ColMajorOut[8] = dZdH;
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
  static void toXYZLocal(T lonRadians, T latRadians, T altitudeMeters0,
                         T *xyz3MetresOut, T *dlon1, T *dlat1) {

    // altitudeMeters0 being nan means that it is undefined.
    // Then it is natural to assume that we are at sea level.
    T altitudeMeters = toFinite(altitudeMeters0, 0.0);

    T latRad = latRadians;
    T lonRad = lonRadians;
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
    t9 = t8+altitudeMeters;
    t11 = t9*coslat;
    t13 = t11*sinlon;
    t16 = a/t6/t5;
    t17 = t16*e2;
    t18 = coslat*coslat;
    t19 = sinlat*t18;
    t23 = t9*sinlat;
    t26 = t11*coslon;
    t31 = 1.0-e2;
    t36 = t8*t31+altitudeMeters;
    t37 = t17*t19;

    if (dlon1) {
      auto values = Array<T>{t3, t4, t5, t6, t8, t9, t11, t13, t16, t17, t18, t19, t23, t26, t31, t36, t37};
      *dlon1 = sqrt(t13*t13 + t26*t26);
    }

    if (dlat1) {
      T u = (t37*coslon-t23*coslon);
      T v = (t37*sinlon-t23*sinlon);
      T w = (t16*t31*t4*coslat+t36*coslat);
      *dlat1 = sqrt(u*u + v*v + w*w);
    }

    if (xyz3MetresOut) {
      xyz3MetresOut[0] = t26;
      xyz3MetresOut[1] = t13;
      xyz3MetresOut[2] = t36*sinlat;
    }
  }
private:
  // Maps lon lat and altitude to an XYZ position in an ECEF coordinate system.
  static void toXYZ(T lonRad, T latRad, T altitudeMetres,
                         T *xyz3MetresOut) {
    toXYZWithJ(lonRad, latRad, altitudeMetres, xyz3MetresOut, nullptr);
  }

  static void posAndDirToXYZ(T lonRad, T latRad, T altitudeMetres, T dirRad, T *xyz3MetresOut, T *xyzDirUnitVectorOut) {
    T J[9];
    toXYZWithJ(lonRad, latRad, altitudeMetres, xyz3MetresOut, J);
    T *eastAxis = J + 0;
    T *northAxis = J + 3;
    normalizeInPlace<T>(3, eastAxis);
    normalizeInPlace<T>(3, northAxis);

    T northCoef = cos(dirRad);
    T eastCoef = sin(dirRad);

    T len2 = 0.0;
    for (int i = 0; i < 3; i++) {
      xyzDirUnitVectorOut[i] = northCoef*northAxis[i] + eastCoef*eastAxis[i];
    }
  }
};

template <typename T>
Length<T> computeDistance(const GeographicPosition<T> &a, const GeographicPosition<T> &b) {
  Length<T> aPos[3], bPos[3];
  WGS84<T>::toXYZ(a, aPos);
  WGS84<T>::toXYZ(b, bPos);
  T dist = T(0.0);
  for (int i = 0; i < 3; i++) {
    dist = dist + sqr(aPos[i].meters() - bPos[i].meters());
  }
  return Length<T>::meters(sqrt(dist));
}



}



#endif /* WGS84TOXYZ_H_ */
