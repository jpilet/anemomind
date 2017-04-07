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
  int rows = 2*n;
  _A = Eigen::MatrixXd::Zero(rows, n);
  _B = Eigen::MatrixXd::Zero(rows, rightDim());
  CHECK(_left.minv() <= _right.maxv());
  CHECK(_left.maxv() <= _right.maxv());
  CHECK(_left.minv() == 0);
}

int BoundaryIndices::totalDim() const {
  return _left.size() + _right.size() - overlap();
}

int BoundaryIndices::overlap() const {
  return std::max(0, _left.maxv() - _right.minv());
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
  if (i < il) {
    return i - _ep;
  } else {
    int localIndex = (i - std::max(_left.maxv(), _right.minv()));
    int offset = (il - _ep);
    return localIndex + offset;
  }

}

BoundaryIndices::Solution BoundaryIndices::solve() const {
  Eigen::MatrixXd X = _A.householderQr().solve(_B);

  int from = _ep;
  int to = std::min(_left.maxv(), _right.maxv() - _ep);
  int k = to - from;
  return Solution{
    X.block(0, 0, _ep, k),
    X.block(_ep, _B.cols() - k, _ep, k)
  };
}

void BoundaryIndices::add(int k, int *inds, double *weights) {
  CHECK(k == _left.size());
  for (int i = 0; i < k; i++) {
    int index = inds[i];
    if (isInnerIndex(index)) {
      int col = computeBCol(index);
      _B(_counter, col) = -weights[i];
    } else {
      int col = computeACol(index);
      _A(_counter, col) = weights[i];
    }
  }
  _counter++;
}


} /* namespace sail */
