/*
 * common.h
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_COMMON_H_
#define SERVER_NAUTICAL_COMMON_H_

namespace sail {

template <typename T>
T computeCurrentFromBoatMotion(
    const T &boatMotionOverWater,
    const T &boatMotionOverGround) {
  return boatMotionOverGround - boatMotionOverWater;
}

template <typename T>
HorizontalMotion<T> computeTrueFlowFromBoatMotionAndApparentFlow(
    const HorizontalMotion<T> &boatMotion,
    const HorizontalMotion<T> &apparentFlow) {
  return boatMotion + apparentFlow;
}

template <typename T>
HorizontalMotion<T> computeApparentWind(const Angle<T> &heading,
    const Angle<T> &awa, const Velocity<T> &aws) {
  return -HorizontalMotion<T>::polar(aws, heading + awa);
}

template <typename T>
Angle<T> computeTwdirFromTrueWind(const HorizontalMotion<T> &tw) {
  return HorizontalMotion<T>(-tw).angle();
}

template <typename T>
Angle<T> computeTwaFromTrueWind(
    const Angle<T> &heading,
    const HorizontalMotion<T> &tw) {
  return computeTwdirFromTrueWind(tw) - heading;
}

}



#endif /* SERVER_NAUTICAL_COMMON_H_ */
