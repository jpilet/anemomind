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
  Eigen::Matrix<T, 3, 1> mastVector() const {
    return Eigen::Matrix<T, 3, 1>(T(0.0), T(0.0), T(1.0));
  }
private:
  GeographicPosition<T> _positions;
  HorizontalMotion<T> _gpsMotion;
  HorizontalMotion<T> _windOverGround;
  HorizontalMotion<T> _currentOverGround;
  Eigen::Matrix<T, 3, 1> _orientationBoatToHorizontalPlane;
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_BOATSTATE_H_ */
