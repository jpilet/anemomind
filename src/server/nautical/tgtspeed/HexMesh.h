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

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
  Eigen::Vector2d _xBasis, _yBasis;
  int _size = 0;
  double _triangleSize = 0;
};


} /* namespace sail */

#endif /* SERVER_NAUTICAL_TGTSPEED_HEXMESH_H_ */
