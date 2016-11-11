/*
 * ECEF.h
 *
 *  Created on: 30 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_ECEF_H_
#define SERVER_NAUTICAL_ECEF_H_

#include <server/math/JetUtils.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <Eigen/Dense>
#include <server/nautical/GeographicPosition.h>

namespace sail {

// Implementation of
// https://microem.ru/files/2012/08/GPS.G1-X-00006.pdf

template <typename T, int order = 0>
struct ECEFCoords {
  TimeDerivative<Length<T>, order> xyz[3];
};

template <typename T, int order = 0>
struct LLACoords {
  TimeDerivative<Angle<T>, order> lon;
  TimeDerivative<Angle<T>, order> lat;
  TimeDerivative<Length<T>, order> height;
};


struct ECEF {
  static constexpr double a = 6378137;
  static constexpr double a2 = a*a;
  static constexpr double f = 1.0/298.257223563;
  static constexpr double b = a*(1.0 - f);
  static constexpr double b2 = b*b;
  static constexpr double e2 = (a2 - b2)/a2;
  static constexpr double ep2 = (a2 - b2)/b2;

  template <typename T>
  static T computeNFromSinPhi2(T sinPhi2) {
    return MakeConstant<T>::apply(a)/
            sqrt(MakeConstant<T>::apply(1.0)
                - MakeConstant<T>::apply(e2)*sinPhi2);
  }

  template <typename T>
  static T computeNFromPhi(T phi) {
    T sinPhi = sin(phi);
    return computeNFromSinPhi2<T>(sinPhi*sinPhi);
  }

  template <typename T>
  static LLACoords<T, 0> geo2lla(
      const GeographicPosition<T> &src) {
    return LLACoords<T, 0>{
      src.lon(),
      src.lat(),
      src.alt()
    };
  }

  template <typename T>
  static ECEFCoords<T> convert(const LLACoords<T> &src) {
    T x, y, z;
    lla2ecef<T>(src.lon.radians(),
        src.lat.radians(), src.height.meters(),
        &x, &y, &z);
    return ECEFCoords<T>{
      Length<T>::meters(x),
      Length<T>::meters(y),
      Length<T>::meters(z),
    };
  }

  template <typename T>
  static ECEFCoords<T> convert(const GeographicPosition<T> &x) {
    return convert(LLACoords<T>{x.lon(), x.lat(), x.alt()});
  }

  template <typename T>
  static LLACoords<T> convert(const ECEFCoords<T> &src) {
    T lon, lat, height;
    ecef2lla<T>(
        src.xyz[0].meters(),
        src.xyz[1].meters(),
        src.xyz[2].meters(),
        &lon, &lat, &height);
    return LLACoords<T>{
      Angle<T>::radians(lon),
      Angle<T>::radians(lat),
      Length<T>::meters(height)
    };
  }

  template <typename T>
  static void lla2ecef(T lonRad, T latRad, T heightMeters,
      T *xMetersOut, T *yMetersOut, T *zMetersOut) {
    T phi = latRad;
    T lambda = lonRad;
    T sinPhi = sin(phi);
    T sinPhi2 = sinPhi*sinPhi;
    T N = computeNFromSinPhi2<T>(sinPhi2);
    T factor = (N + heightMeters)*cos(phi);
    *xMetersOut = factor*cos(lambda);
    *yMetersOut = factor*sin(lambda);
    *zMetersOut = (MakeConstant<T>::apply(b2/a2)*N
        + heightMeters)*sinPhi;
  }

  template <typename T>
  static void ecef2lla(
      T xMeters, T yMeters, T zMeters,
      T *lonRadOut, T *latRadOut, T *heightMetersOut) {
    T lambda = atan2(yMeters, xMeters);
    T p = sqrt(xMeters*xMeters + yMeters*yMeters);
    T theta = atan2(
        zMeters*MakeConstant<T>::apply(a),
        p*MakeConstant<T>::apply(b));
    T sinTheta = sin(theta);
    T sin3Theta = sinTheta*sinTheta*sinTheta;
    T cosTheta = cos(theta);
    T cos3Theta = cosTheta*cosTheta*cosTheta;
    T phi = atan2(
        zMeters + MakeConstant<T>::apply(ep2*b)*sin3Theta,
        p - MakeConstant<T>::apply(e2*a)*cos3Theta);
    *heightMetersOut = p/cos(phi) - computeNFromPhi<T>(phi);
    *lonRadOut = lambda;
    *latRadOut = phi;
  }

  template <typename T>
  static Eigen::Matrix<T, 3, 3> makeEcefToHMotionMatrix(
      T lon, T lat, T h) {
    T phi = lat;
    T lambda = lon;
    Eigen::Matrix<T, 3, 3> A;
    A << -sin(phi)*cos(lambda),
         -sin(phi)*sin(lambda),
         cos(phi),

         -sin(lambda), cos(lambda), MakeConstant<T>::apply(0.0),

         -cos(phi)*cos(lambda),
         -cos(phi)*sin(lambda),
         -sin(phi);
    return A;
  }

  template <typename T>
  static Eigen::Matrix<T, 3, 3> makeEcefToHMotionMatrix(
      const LLACoords<T> &lla) {
    return makeEcefToHMotionMatrix(
        lla.lon.radians(),
        lla.lat.radians(),
        lla.height.meters());
  }

  template <typename T>
  static Eigen::Matrix<T, 3, 1> hMotionToXYZ(
      T lon, T lat, T h,
      T eastMps, T northMps, T downMps) {
    Eigen::Matrix<T, 3, 1> B(northMps, eastMps, downMps);
    auto A = makeEcefToHMotionMatrix(lon, lat, h);
    return A.lu().solve(B);
  }



  template <typename T>
  static Eigen::Matrix<T, 3, 1> computeNorthEastDownMotion(
      const ECEFCoords<T, 0> &pos,
      const ECEFCoords<T, 1> &motion) {
    auto A = makeEcefToHMotionMatrix(convert(pos));
    Eigen::Matrix<T, 3, 1> B(
        motion.xyz[0].metersPerSecond(),
        motion.xyz[1].metersPerSecond(),
        motion.xyz[2].metersPerSecond());
    Eigen::Matrix<T, 3, 1> hmotion = A*B;
    return hmotion;
  }


  template <typename T>
  static HorizontalMotion<T> computeEcefToHMotion(
      const ECEFCoords<T, 0> &pos,
      const ECEFCoords<T, 1> &motion) {
    auto hmotion = computeNorthEastDownMotion(pos, motion);
    return HorizontalMotion<T>(
        Velocity<T>::metersPerSecond(hmotion(1)),
        Velocity<T>::metersPerSecond(hmotion(0)));
  }

  template <typename T>
  static ECEFCoords<T, 1> hMotionToXYZ(
      const LLACoords<T> &pos,
      const HorizontalMotion<T> &motion,
      Length<T> height = Length<T>::meters(MakeConstant<T>::apply(0.0))) {
    auto xyz = hMotionToXYZ(
        pos.lon.radians(), pos.lat.radians(), pos.height.meters(),
        motion[0].metersPerSecond(),
        motion[1].metersPerSecond(),
        height.meters());
    return ECEFCoords<T, 1>{
      Velocity<T>::metersPerSecond(xyz(0)),
      Velocity<T>::metersPerSecond(xyz(1)),
      Velocity<T>::metersPerSecond(xyz(2))
    };
  }
};


}

#endif /* SERVER_NAUTICAL_ECEF_H_ */
