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
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {
namespace CornerCalibTestData {

template <typename T>
HorizontalMotion<T> rotate(T radians, const HorizontalMotion<T> &v) {
  T cosv = cos(radians);
  T sinv = sin(radians);
  return HorizontalMotion<T>(
      cosv*v[0] - sinv*v[1],
      sinv*v[0] + cosv*v[1]);
}

template <typename T>
HorizontalMotion<T> correctOrCorruptVector(
    const HorizontalMotion<double> &w,
    T bias, T speedOffset, T angleOffset) {
  Eigen::Vector2d wvec(w[0].knots(), w[1].knots());
  Eigen::Vector2d what = wvec.normalized();
  auto v = Velocity<T>::knots(speedOffset);
  return bias*rotate(angleOffset, w.cast<T>())
    + rotate(angleOffset,
        HorizontalMotion<T>(T(what(0))*v, T(what(1))*v));
}

struct TestSample {
  HorizontalMotion<double> boatMotionVec;
  HorizontalMotion<double> groundTruthCurrentMotionVec;
  Arrayd params;

  // currentMotion = gpsMotion - motionOverWater <=>
  // motionOverWater = gpsMotion - currentMotion
  HorizontalMotion<double> groundTruthMotionOverWaterVec() const {
    return boatMotionVec - groundTruthCurrentMotionVec;
  }
  HorizontalMotion<double> corruptedMotionOverWaterVec() const;

  HorizontalMotion<double> refMotion() const {
    return boatMotionVec;
  }
};

HorizontalMotion<double> getTrueConstantCurrent();

Array<TestSample> makeTestSamples(Arrayd params);

Arrayd getDefaultCorruptParams();

}
}

#endif /* SERVER_NAUTICAL_CALIB_CORNERCALIBTESTDATA_H_ */
