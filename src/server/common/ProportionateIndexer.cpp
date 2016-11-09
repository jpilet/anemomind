/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ProportionateIndexer.h"
#include <cassert>
#include <cmath>
#include <server/common/Functional.h>

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
  return toArray(map(proportions(), [&](double x) {return x == 0;}));
}

Arrayb ProportionateIndexer::remaining() const {
  return toArray(map(proportions(), [&](double x) {return x != 0;}));
}


double ProportionateIndexer::sumTo(double fracIndex) const {
  double indexd = floor(fracIndex);
  double acc = fracIndex - indexd;
  int index = int(indexd);
  if (index >= size()) {
    return sum();
  }
  if (index < 0) {
    return 0;
  }

  while (0 < index) {
    int next = parent(index);
    if (rightChild(next) == index) {
      acc += _values[leftChild(next)];;
    }
    index = next;
  }
  return acc;
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

ProportionateIndexer::LookupResult ProportionateIndexer::getAdvanced(int nodeIndex, double localX, double initX) const {
  assert(0 <= localX);
  assert(localX <= _values[nodeIndex]);
  if (isLeaf(nodeIndex)) {
    int proportionIndex = nodeIndex - _offset;
    return LookupResult(proportionIndex, localX, initX);
  } else {
    int left = leftChild(nodeIndex);
    double leftSum = _values[left];
    if (localX < leftSum) {
      return getAdvanced(left, localX, initX);
    } else {
      return getAdvanced(rightChild(nodeIndex), localX - leftSum, initX);
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

void ProportionateIndexer::assign(int index0, double newValue) {
  int start = index0 + _offset;

  // Initialize the leaf node
  _values[start] = newValue;

  // Loop over inner nodes
  int index = parent(start);
  while (index != -1) {
    _values[index] = _values[leftChild(index)]
                   + _values[rightChild(index)];
    index = parent(index);
  }
}

void ProportionateIndexer::remove(int index) {
  assign(index, 0);
}

Arrayd ProportionateIndexer::proportions() const {
  return _values.slice(_offset, _offset + _count);
}

int ProportionateIndexer::getAndRemove(double x) {
  int index = get(x).index;
  remove(index);
  return index;
}

} /* namespace mmm */
