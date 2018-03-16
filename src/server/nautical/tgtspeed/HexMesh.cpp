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
  : _size(size), _triangleSize(triangleSize),
    _gridSize(2*size + 1) {
  _xBasis = Eigen::Vector2d(triangleSize, 0.0);
  _yBasis = Eigen::Vector2d(0.5*triangleSize, 0.5*sqrt(3)*triangleSize);
  _offset = size*Eigen::Vector2d(1.0, 1.0);
  CHECK(fabs((_xBasis - _yBasis).norm() - triangleSize) < 0.001);

  for (int i = 0; i < 2; i++) {
    _g2w(i, 0) = _xBasis(i);
    _g2w(i, 1) = _yBasis(i);
  }
  _w2g = _g2w.inverse();
}


} /* namespace sail */
