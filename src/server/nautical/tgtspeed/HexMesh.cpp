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
#include <server/common/ArrayBuilder.h>
#include <iostream>
#include <iomanip>
#include <server/transducers/Transducer.h>

namespace sail {

bool HexMesh::isValidIntegerCoordinate(int x, int y) const {
  auto xy = x + y;
  return 0 <= x && x < _gridSize && 0 <= y && y < _gridSize
      && _size <= xy && xy <= 3*_size;
}

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
  _grid = MDArray<int, 2>(_gridSize, _gridSize);
  _grid.setAll(-1);

  ArrayBuilder<std::pair<int, int>> _vertexToCoordBuilder;
  int _counter = 0;
  for (int y = 0; y < _gridSize; y++) {
    for (int x = 0; x < _gridSize; x++) {
      if (isValidIntegerCoordinate(x, y)) {
        _grid(x, y) = _counter;
        _vertexToCoordBuilder.add({x, y});
        _counter++;
      }
    }
  }
  _index2coord = _vertexToCoordBuilder.get();
}

void HexMesh::dispVertexLayout() {
  for (int y = 0; y < _gridSize; y++) {
    for (int x = 0; x < _gridSize; x++) {
      std::cout << std::setw(3) << _grid(x, _gridSize-1-y) << " ";
    }
    std::cout << std::endl;
  }
}

int HexMesh::vertexIndexAt(int x, int y) const {
  return isValidIntegerCoordinate(x, y)?
      _grid(x, y) : -1;
}

Optional<std::array<WeightedIndex, 3>> HexMesh::represent(
    const Eigen::Vector2d& pos) const {
  auto gridPos = worldToGrid(pos);

  auto xf = floor(gridPos(0));
  int x = int(xf);
  auto yf = floor(gridPos(1));
  int y = int(yf);

  auto lower = (gridPos(0) - xf) + (gridPos(1) - yf) < 1;
  std::array<int, 3> inds;
  /*inds[0] =
    vertexIndexAt(x+1, y),
    vertexIndexAt(x, y+1),
    lower? vertexIndexAt(x, y) : vertexIndexAt(x+1, y+1)
  };*/
  if (transduce(inds, trMap([](int i) {return i == -1;}), IntoOr())) {

  }
}


} /* namespace sail */
