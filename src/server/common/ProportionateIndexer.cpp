/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ProportionateIndexer.h"
#include <cassert>

namespace sail {

Arrayd ProportionateIndexer::prepare(int count) {
  int leftmost = 0;
  int rightmost = 0;
  _count = count;
  while (rightmost - leftmost + 1 < _count) {
    leftmost = leftChild(leftmost);
    rightmost = rightChild(rightmost);
  }
  _offset = leftmost;
  _values = Arrayd::fill(rightmost + 1, 0.0);
  return _values.slice(leftmost, leftmost + _count);
}


ProportionateIndexer::ProportionateIndexer(Arrayd proportions) {
  proportions.copyToSafe(prepare(proportions.size()));
  fillInnerNodes(0);
}

ProportionateIndexer::ProportionateIndexer(int count,
    std::function<double(int)> widthPerProp) {
  Arrayd dst = prepare(count);
  for (int i = 0; i < count; i++) {
    dst[i] = widthPerProp(i);
  }
  fillInnerNodes(0);
}


Arrayb ProportionateIndexer::selected() const {
  return proportions().map<bool>([&](double x) {return x == 0;});
}

Arrayb ProportionateIndexer::remaining() const {
  return proportions().map<bool>([&](double x) {return x != 0;});
}


double ProportionateIndexer::fillInnerNodes(int root) {
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

ProportionateIndexer::LookupResult ProportionateIndexer::getBySum(int node, double cumulative, double x) const {
  assert(0 <= x);
  assert(x <= _values[node]);
  if (isLeaf(node)) {
    return LookupResult(node - _offset, cumulative, x);
  } else {
    int left = leftChild(node);
    double leftSum = _values[left];
    if (x < leftSum) {
      return getBySum(left, cumulative, x);
    } else {
      return getBySum(rightChild(node), cumulative + leftSum, x - leftSum);
    }
  }
}

int ProportionateIndexer::parent(int index) {
  if (index <= 0) {
    return -1;
  }
  return (index - 1)/2;
}

int ProportionateIndexer::leftChild(int index) {
  return 2*index + 1;
}

int ProportionateIndexer::rightChild(int index) {
  return 2*index + 2;
}

int ProportionateIndexer::get(double x) const {
  assert(0 <= x);
  assert(x <= 1.0);
  return getBySum(0, 0, x*sum()).index;
}

void ProportionateIndexer::assign(int index0, double newValue) {
  int start = index0 + _offset;
  int index = start;
  double change = newValue - _values[index];
  while (index != -1) {
    _values[index] += change;
    index = parent(index);
  }
  _values[start] = newValue; // Make sure it is exactly the new value
}

void ProportionateIndexer::remove(int index) {
  assign(index, 0);
}

Arrayd ProportionateIndexer::proportions() const {
  return _values.slice(_offset, _offset + _count);
}

int ProportionateIndexer::getAndRemove(double x) {
  int index = get(x);
  remove(index);
  return index;
}

} /* namespace mmm */
