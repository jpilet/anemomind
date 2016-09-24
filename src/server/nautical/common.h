/*
 * common.h
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_COMMON_H_
#define SERVER_NAUTICAL_COMMON_H_

#include <server/math/JetUtils.h>

namespace sail {

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
Angle<T> angleFrom(const Angle<T> &angle) {
  return angle - Angle<T>::degrees(MakeConstant<T>::apply(180.0));
}

template <typename T>
Optional<Angle<T> > computeAWA(const HorizontalMotion<T> &apparentWind,
                    Angle<T> heading) {
  auto alpha = apparentWind.optionalAngle();
  return alpha.defined()?
    angleFrom(alpha.get()) - heading
    : Optional<Angle<T> >();
}

template <typename T>
Angle<T> computeAWA(const HorizontalMotion<T> &boatOverGround,
                    const HorizontalMotion<T> &windOverGround,
                    const Angle<T> &heading) {
  return angleFrom(computeApparentWind(
      boatOverGround, windOverGround).angle()) - heading;
}


}



#endif /* SERVER_NAUTICAL_COMMON_H_ */
