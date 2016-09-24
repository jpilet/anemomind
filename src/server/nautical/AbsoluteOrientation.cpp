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
Eigen::Matrix3d boxAnglesToRotation(
    const AbsoluteOrientation &orientation) {
  Eigen::Matrix3d R0 = computeRotationFromOmega<double>(
      -orientation.heading.radians()*Eigen::Vector3d(0, 1, 0));
  Eigen::Matrix3d R1 = computeRotationFromOmega<double>(
      -orientation.pitch.radians()*Eigen::Vector3d(1, 0, 0));
  Eigen::Matrix3d R2 = computeRotationFromOmega<double>(
      orientation.roll.radians()*Eigen::Vector3d(0, 0, 1));
  return R2*R1*R0;
}

}




