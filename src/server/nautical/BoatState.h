/*
 * BoatState.h
 *
 *  Created on: 25 Aug 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_BOATSTATE_H_
#define SERVER_NAUTICAL_BOATSTATE_H_

#include <Eigen/Dense>

namespace sail {

template <typename T>
class BoatState {
public:
private:
  GeographicPosition<T> _positions;
  HorizontalMotion<T> _gpsMotion;
  HorizontalMotion<T> _windOverGround;
  HorizontalMotion<T> _currentOverGround;
  Eigen::Matrix<T, 3, 1> _orientation;
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_BOATSTATE_H_ */
