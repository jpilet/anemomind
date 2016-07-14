/*
 * common.h
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_COMMON_H_
#define SERVER_NAUTICAL_COMMON_H_

template <typename T>
T computeCurrentFromBoatMotion(
    const T &boatMotionOverWater,
    const T &boatMotionOverGround) {
  return boatMotionOverGround - boatMotionOverWater;
}


#endif /* SERVER_NAUTICAL_COMMON_H_ */
