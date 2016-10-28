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
  _left(left), _right(right), _ep(ep), _counter(0) {
  int n = varDim();
  _A = Eigen::MatrixXd::Zero(n, n);
  _B = Eigen::MatrixXd::Zero(n, rightDim());
}

int BoundaryIndices::totalDim() const {
  auto overlap = std::max(0, _left.maxv() - _right.minv());
  return _left.size() + _right.size() - overlap;
}

bool BoundaryIndices::isLeftIndex(int i) const {
  return i < _ep;
}

int BoundaryIndices::rightDim() const {
  return totalDim() - varDim();
}

int BoundaryIndices::epRight() const {
  return _right.maxv() - _ep;
}

int BoundaryIndices::innerLimit() const {
  return std::max(_left.maxv(), _right.minv());
}

bool BoundaryIndices::isRightIndex(int i) const {
  return epRight() <= i;
}

bool BoundaryIndices::isInnerIndex(int i) const {
  return !isLeftIndex(i) && !isRightIndex(i);
}

int BoundaryIndices::computeACol(int i) const {
  CHECK(!isInnerIndex(i));
  if (isLeftIndex(i)) {
    return i;
  }
  return i - epRight() + _ep;
}

int BoundaryIndices::computeBCol(int i) const {
  auto il = _left.maxv();
  return i < il? i - _ep : (i - _right.minv()) + (il - _ep);
}

BoundaryIndices::Solution BoundaryIndices::solve() const {
  CHECK(_counter == varDim());
  Eigen::MatrixXd X = _A.lu().solve(_B);
  int k = _left.maxv() - _ep;
  return Solution{
    X.block(0, 0, k, X.cols()),
    X.block(k, 0, k, X.cols())
  };
}

void BoundaryIndices::add(int k, int *inds, double *weights) {
  CHECK(k == _left.size());
  for (int i = 0; i < k; i++) {
    int index = inds[i];
    if (isInnerIndex(index)) {
      _A(_counter, computeACol(index)) = weights[i];
    } else {
      _B(_counter, computeBCol(index)) = -weights[i];
    }
  }
  _counter++;
}


} /* namespace sail */
