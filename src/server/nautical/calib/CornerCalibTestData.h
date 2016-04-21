/*
 * CornerCalibTestData.h
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_CORNERCALIBTESTDATA_H_
#define SERVER_NAUTICAL_CALIB_CORNERCALIBTESTDATA_H_

#include <Eigen/Dense>
#include <server/common/Array.h>

namespace sail {
namespace CornerCalibTestData {

template <typename T>
Eigen::Matrix<T, 2, 1> rotate(T radians, const Eigen::Vector2d &v) {
  T cosv = cos(radians);
  T sinv = sin(radians);
  return Eigen::Matrix<T, 2, 1>(cosv*v[0] - sinv*v[1], sinv*v[0] + cosv*v[1]);
}

template <typename T>
Eigen::Matrix<T, 2, 1> correctOrCorruptVector(const Eigen::Vector2d &w,
    T bias, T speedOffset, T angleOffset) {
  return bias*rotate(angleOffset, w)
    + speedOffset*rotate(angleOffset, w.normalized());
}

struct TestSample {
  Eigen::Vector2d boatMotionVec;
  Eigen::Vector2d groundTruthCurrentMotionVec;
  Arrayd params;

  // currentMotion = gpsMotion - motionOverWater <=>
  // motionOverWater = gpsMotion - currentMotion
  Eigen::Vector2d groundTruthMotionOverWaterVec() const {
    return boatMotionVec - groundTruthCurrentMotionVec;
  }
  Eigen::Vector2d corruptedMotionOverWaterVec() const;

  Eigen::Vector2d refMotion() const {
    return boatMotionVec;
  }
};

Eigen::Vector2d getTrueConstantCurrent();

Array<TestSample> makeTestSamples(Arrayd params);

Arrayd getDefaultCorruptParams();

}
}

#endif /* SERVER_NAUTICAL_CALIB_CORNERCALIBTESTDATA_H_ */
