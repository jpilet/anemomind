/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ProportionateSampler.h"
#include <cassert>

namespace sail {

ProportionateSampler::ProportionateSampler(Arrayd proportions) {
  int leftmost = 0;
  int rightmost = 0;
  int len = proportions.size();
  while (rightmost - leftmost + 1 < len) {
    leftmost = leftChild(leftmost);
    rightmost = rightChild(rightmost);
  }
  _offset = leftmost;
  _values = Arrayd::fill(rightmost + 1, 0.0);
  proportions.copyToSafe(_values.slice(leftmost, leftmost + len));
  fillInnerNodes(0);
}

double ProportionateSampler::fillInnerNodes(int root) {
  if (isLeaf(root)) {
    return _values[root];
  } else {
    double left = fillInnerNodes(leftChild(root));
    double right = fillInnerNodes(rightChild(root));
    double sum = left + right;
    _values[root] = sum;
    return sum;
  }
}

int ProportionateSampler::getBySum(int node, double x) const {
  assert(0 <= x);
  assert(x <= _values[node]);
  if (isLeaf(node)) {
    return node - _offset;
  } else {
    int left = leftChild(node);
    double leftSum = _values[left];
    if (x < leftSum) {
      return getBySum(left, x);
    } else {
      return getBySum(rightChild(node), x - leftSum);
    }
  }
}

int ProportionateSampler::parent(int index) {
  if (index <= 0) {
    return -1;
  }
  return (index - 1)/2;
}

int ProportionateSampler::leftChild(int index) {
  return 2*index + 1;
}

int ProportionateSampler::rightChild(int index) {
  return 2*index + 2;
}

int ProportionateSampler::get(double x) const {
  assert(0 <= x);
  assert(x <= 1.0);
  return getBySum(0, x*_values[0]);
}

void ProportionateSampler::remove(int index0) {
  int start = index0 + _offset;
  int index = start;
  double v = _values[index];
  while (index != -1) {
    _values[index] -= v;
    index = parent(index);
  }
  _values[start] = 0; // Make sure it is exactly 0.
}

int ProportionateSampler::getAndRemove(double x) {
  int index = get(x);
  remove(index);
  return index;
}

} /* namespace mmm */
