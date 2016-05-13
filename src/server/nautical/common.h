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

/*
 * It seems that "True wind" often means wind over water, and not wind over ground.
 * So it makes sense to avoid this term in order to avoid confusion, and maybe use
 * slightly more verbose "wind over ground" and "wind over water" or something...
 */

template <typename T>
HorizontalMotion<T> computeCurrentFromBoatMotionOverWaterAndGround(
    const HorizontalMotion<T> &boatMotionOverWater,
    const HorizontalMotion<T> &boatMotionOverGround) {
  return boatMotionOverGround - boatMotionOverWater;
}

template <typename T>
HorizontalMotion<T> computeWindOverGroundFromApparentWindAndBoatMotion(
    const HorizontalMotion<T> &apparentWind,
    const HorizontalMotion<T> &boatMotionOverGround) {
  return boatMotionOverGround + apparentWind;
}

template <typename T>
Angle<T> computeTwdirFromWindOverGround(const HorizontalMotion<T> &windOverGround) {
  return windOverGround.angle() + Angle<double>::degrees(180.0);
}

template <typename T>
Angle<T> computeTwaFromTwdirAndHeading(Angle<T> twdir, Angle<T> heading) {
  return twdir - heading;
}

}




#endif /* SERVER_NAUTICAL_COMMON_H_ */
