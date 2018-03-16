/*
 * HexMesh.h
 *
 *  Created on: 16 Mar 2018
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_TGTSPEED_HEXMESH_H_
#define SERVER_NAUTICAL_TGTSPEED_HEXMESH_H_

#include <Eigen/Dense>
#include <server/common/MDArray.h>
#include <server/common/WeightedIndex.h>
#include <server/common/Optional.h>
#include <array>

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

  bool isValidIntegerCoordinate(int x, int y) const;

  void dispVertexLayout();

  int vertexCount() const {return _index2coord.size();}

  Optional<std::array<WeightedIndex, 3>> represent(
      const Eigen::Vector2d& pos) const;

  int vertexIndexAt(int x, int y) const;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
  Eigen::Vector2d _xBasis, _yBasis, _offset;
  Eigen::Matrix2d _g2w, _w2g;
  int _size = 0;
  int _gridSize = 0;
  double _triangleSize = 0;
  MDArray<int, 2> _grid;
  Array<std::pair<int, int>> _index2coord;
};


} /* namespace sail */

#endif /* SERVER_NAUTICAL_TGTSPEED_HEXMESH_H_ */
