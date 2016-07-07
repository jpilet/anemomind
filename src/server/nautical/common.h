/*
 * common.h
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_COMMON_H_
#define SERVER_NAUTICAL_COMMON_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

template <typename T>
T computeCurrentFromBoatMotion(
    const T &boatMotionOverWater,
    const T &boatMotionOverGround) {
  return boatMotionOverGround - boatMotionOverWater;
}

template <typename T>
Angle<T> computeTwdirFromTrueWind(const HorizontalMotion<T> &tw) {
  return tw.angle() + Angle<double>::degrees(180.0);
}


}


#endif /* SERVER_NAUTICAL_COMMON_H_ */
