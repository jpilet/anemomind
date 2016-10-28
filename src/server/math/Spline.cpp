/*
 * Spline.cpp
 *
 *  Created on: 12 Oct 2016
 *      Author: jonas
 */

#include "Spline.h"
#include <cassert>
#include <initializer_list>

namespace sail {

BoundaryIndices::BoundaryIndices(Spani left, Spani right, int ep) :
  _left(left), _right(right), _ep(ep) {}

int BoundaryIndices::totalDim() const {
  auto overlap = std::max(0, _left.maxv() - _right.minv());
  return _left.size() + _right.size() - overlap;
}

bool BoundaryIndices::isLeftIndex(int i) const {
  return i < _ep;
}

bool BoundaryIndices::isRightIndex(int i) const {
  return _right.maxv() - _ep <= i;
}

bool BoundaryIndices::isInnerIndex(int i) const {
  return !isLeftIndex(i) && !isRightIndex(i);
}


} /* namespace sail */
