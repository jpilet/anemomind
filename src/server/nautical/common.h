/*
 * common.h
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_COMMON_H_
#define SERVER_NAUTICAL_COMMON_H_

#include <server/math/JetUtils.h>
#include <Eigen/Dense>

namespace sail {

template <typename T>
Eigen::Matrix<T, 2, 1> makeNauticalUnitVector(Angle<T> x) {
  return Eigen::Matrix<T, 2, 1>(sin(x), cos(x));
}

template <typename T>
Optional<Angle<T>> computeNauticalAngle(
    const Eigen::Matrix<T, 2, 1> &x) {
  T y = atan2(x(0), x(1));
  return isFinite(y)?
      Optional<Angle<T>>(y)
      : Optional<Angle<T>>();
}

template <typename T>
T computeCurrentFromBoatMotion(
    const T &boatMotionOverWater,
    const T &boatMotionOverGround) {
  return boatMotionOverGround - boatMotionOverWater;
}

template <typename T>
HorizontalMotion<T> computeApparentWind(
    const HorizontalMotion<T> &boatOverGround,
    const HorizontalMotion<T> &windOverGround) {
  return windOverGround - boatOverGround;
}

template <typename T>
Angle<T> flipAngle(Angle<T> x) {
  return x + Angle<T>::degrees(MakeConstant<T>::apply(180.0));
}

template <typename T>
Optional<Angle<T> > computeAWA(const HorizontalMotion<T> &apparentWind,
                    Angle<T> heading) {
  auto alpha = apparentWind.optionalAngle();
  return alpha.defined()?
    flipAngle(alpha.get()) - heading
    : Optional<Angle<T> >();
}

template <typename T>
Angle<T> computeAWA(const HorizontalMotion<T> &boatOverGround,
                    const HorizontalMotion<T> &windOverGround,
                    const Angle<T> &heading) {
  return flipAngle(computeApparentWind(
      boatOverGround, windOverGround).angle()) - heading;
}

template <typename T>
HorizontalMotion<T> computeBoatOverWater(
    const HorizontalMotion<T> &boatOverGround,
    const HorizontalMotion<T> &currentOverGround) {
  return boatOverGround - currentOverGround;
}

template <typename T>
HorizontalMotion<T> makeApparentWind(
    Velocity<T> aws, Angle<T> awa, Angle<T> heading) {
  return HorizontalMotion<T>::polar(aws, flipAngle(awa) + heading);
}


}



#endif /* SERVER_NAUTICAL_COMMON_H_ */
