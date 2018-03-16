/*
 * HexMesh.h
 *
 *  Created on: 16 Mar 2018
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_TGTSPEED_HEXMESH_H_
#define SERVER_NAUTICAL_TGTSPEED_HEXMESH_H_

#include <Eigen/Dense>

namespace sail {

class HexMesh {
public:
  HexMesh() {}
  HexMesh(int size, double triangleSize);

  Eigen::Vector2d gridToWorld(const Eigen::Vector2d& coords) const {
    return _g2w*(coords - _offset);
  }

  Eigen::Vector2d worldToGrid(const Eigen::Vector2d& coords) const {
    return _w2g*coords + _offset;
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
  Eigen::Vector2d _xBasis, _yBasis, _offset;
  Eigen::Matrix2d _g2w, _w2g;
  int _size = 0;
  int _gridSize = 0;
  double _triangleSize = 0;
};


} /* namespace sail */

#endif /* SERVER_NAUTICAL_TGTSPEED_HEXMESH_H_ */
