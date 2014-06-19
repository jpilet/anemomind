/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HNodeGroup.h"
#include <server/common/logging.h>
#include <server/common/string.h>
#include <iostream>

namespace sail {

HNodeGroup::HNodeGroup(std::string description, ArrayBuilder<HNodeGroup> subnodes) :
      _children(subnodes.get()), _desc(description), _index(-1) {}

HNodeGroup::HNodeGroup(int index, std::string description, ArrayBuilder<HNodeGroup> subnodes) :
  _children(subnodes.get()), _desc(description), _index(index) {}

HNodeGroup::HNodeGroup(int index, std::string desc) : _index(index), _desc(desc) {}

int HNodeGroup::checkIndexedAndCountNodes(Arrayb marked) {
  int count = 1; // for this node
  if (_index != -1) {
    if (marked[_index]) {
      LOG(FATAL) << stringFormat("Index %d has already been marked", _index);
    }
    marked[_index] = true;
  }
  for (auto c : _children) {
    count += c.checkIndexedAndCountNodes(marked);
  }
  return count;
}

int HNodeGroup::fillNodes(Array<HNode> nodes, int counter, int parent, const char *fmt) {
  int index = _index;
  if (_index == -1) {
    index = counter;
    counter++;
  }
  nodes[index] = HNode(index, parent,
      stringFormat(fmt, index), _desc);

  for (auto c : _children) {
    counter = c.fillNodes(nodes, counter, index, fmt);
  }
  return counter;
}

Array<HNode> HNodeGroup::compile(const char *fmt) {
  int maxIndex = getMaxIndex();
  int indexedNodeCount = maxIndex+1;
  Arrayb marked = Arrayb::fill(indexedNodeCount, false);
  int totalNodeCount = checkIndexedAndCountNodes(marked);

  // Make sure that all explicit indices are compact:
  CHECK(all(marked));

  Array<HNode> nodes(totalNodeCount);
  int finalCount = fillNodes(nodes, indexedNodeCount, -1, fmt);
  CHECK(finalCount == totalNodeCount);
  return nodes;
}


int HNodeGroup::getMaxIndex() {
  int maxi = _index;
  for (auto c : _children) {
    maxi = std::max(maxi, c.getMaxIndex());
  }
  return maxi;
}

namespace {
  const int initArraySize = 12;
}

ArrayBuilder<HNodeGroup> operator+(HNodeGroup a, HNodeGroup b) {
  ArrayBuilder<HNodeGroup> builder(initArraySize);
  builder.add(a);
  builder.add(b);
  return builder;
}

ArrayBuilder<HNodeGroup> operator+(ArrayBuilder<HNodeGroup> a, HNodeGroup b) {
  a.add(b);
  return a;
}


ArrayBuilder<HNodeGroup> operator+(HNodeGroup a, ArrayBuilder<HNodeGroup> b) {
  b.add(a);
  return b;
}

} /* namespace sail */
