/*
 * HexMesh.cpp
 *
 *  Created on: 16 Mar 2018
 *      Author: jonas
 */

#include "HexMesh.h"
#include <Eigen/Dense>
#include <math.h>
#include <server/common/logging.h>

namespace sail {

HexMesh::HexMesh(int size, double triangleSize)
  : _size(size), _triangleSize(triangleSize) {
  _xBasis = Eigen::Vector2d(triangleSize, 0.0);
  _yBasis = Eigen::Vector2d(0.5*triangleSize, 0.5*sqrt(3)*triangleSize);
  CHECK(fabs((_xBasis - _yBasis).norm() - triangleSize) < 0.001);
}

} /* namespace sail */
