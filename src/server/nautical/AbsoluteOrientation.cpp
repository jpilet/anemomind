/*
 * AbsoluteOrientation.cpp
 *
 *  Created on: Apr 1, 2016
 *      Author: jonas
 */

#include <server/nautical/AbsoluteOrientation.h>
#include <server/common/numerics.h>
#include <server/math/AxisAngle.h>

namespace sail {

// See this src/device/anemobox/anemonode/components/http/static/3d.js
// and the function drawScene()
Eigen::Matrix3d BNO055AnglesToRotation(
    const AbsoluteOrientation &orientation) {

  double s = -1.0;

  Eigen::Matrix3d R0 = computeRotationFromOmega<double>(
      -s*orientation.heading.radians()*Eigen::Vector3d(0, 1, 0));
  Eigen::Matrix3d R1 = computeRotationFromOmega<double>(
      -s*orientation.pitch.radians()*Eigen::Vector3d(1, 0, 0));
  Eigen::Matrix3d R2 = computeRotationFromOmega<double>(
      s*orientation.roll.radians()*Eigen::Vector3d(0, 0, 1));

  return R2*R1*R0;
}

}




