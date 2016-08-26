/*
 * BoatState.h
 *
 *  Created on: 25 Aug 2016
 *      Author: jonas
 *
 * A BoatState represents that best knowledge that we have
 * about a boat at some point in time. All measurements that
 * it contains are corrected, and it does not contain any raw or
 * ambiguous data.
 *
 * A sequence of BoatStates is produced by sail::Reconstructor::reconstruct.
 * The BoatState can be safely consumed by the rest of the application,
 * without any worry that the data is wrong.
 */

#ifndef SERVER_NAUTICAL_BOATSTATE_H_
#define SERVER_NAUTICAL_BOATSTATE_H_

#include <Eigen/Dense>
#include <server/nautical/GeographicPosition.h>

namespace sail {

template <typename T>
class BoatState {
public:
  BoatState(
      const GeographicPosition<T> &position,
      const HorizontalMotion<T> &gpsMotion,
      const HorizontalMotion<T> &windOverGround,
      const HorizontalMotion<T> &currentOverGround,
      const Eigen::Matrix<T, 3, 1> &orientationBoatToLocal) :
        _position(position),
        _gpsMotion(gpsMotion),
        _windOverGround(windOverGround),
        _currentOverGround(currentOverGround),
        _orientationBoatToLocal(orientationBoatToLocal) {}


  // This is how the orthonormal basis attached to the boat
  // relates to the boat. Any one of these vector can be expressed
  // as a cross product of the other two

  // When the boat is in a perfectly horizontal equilibrium, this
  // vector points up, pretty much along the mast.
  static Eigen::Matrix<T, 3, 1> mastVector() {
    return Eigen::Matrix<T, 3, 1>(T(0.0), T(0.0), T(1.0));
  }

  // This vector points in the direction of the heading of the boat.
  // When the boat is horizontal, this vector is parallel to the sea.
  static Eigen::Matrix<T, 3, 1> headingVector() {
    return Eigen::Matrix<T, 3, 1>(T(0.0), T(1.0), T(0.0));
  }

  // This vector points towards starboard, parallel to the sea surface.
  static Eigen::Matrix<T, 3, 1> starboardVector() {
    return Eigen::Matrix<T, 3, 1>(T(1.0), T(0.0), T(0.0));
  }

  GeographicPosition<T> _position;
  HorizontalMotion<T> _gpsMotion;
  HorizontalMotion<T> _windOverGround;
  HorizontalMotion<T> _currentOverGround;
  Eigen::Matrix<T, 3, 1> _orientationBoatToLocal;
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_BOATSTATE_H_ */
