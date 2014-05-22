/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FlowField.h"

namespace sail {

void FlowField::init(Grid3d grid, Array<FlowVector> flowVecs) {
  _grid = grid;
  _flow = flowVecs;
}

FlowField::FlowVector FlowField::map(
    Length<double> x, Length<double> y, Duration<double> time) {
  double localPos[3] = {x.meters(), y.meters(), time.seconds()};
  int inds[Grid3d::WVL];
  double weights[Grid3d::WVL];
  _grid.makeVertexLinearCombination(localPos, inds, weights);
  FlowVector dst{Velocity<double>::metersPerSecond(0), Velocity<double>::metersPerSecond(0)};
  for (int i = 0; i < Grid3d::WVL; i++) {
    dst = dst + _flow[inds[i]].scaled(weights[i]);
  }
  return dst;
}

} /* namespace sail */
