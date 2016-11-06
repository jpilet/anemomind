/*
 * ReduceTree.h
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_REDUCETREE_H_
#define SERVER_COMMON_REDUCETREE_H_

#include <server/common/Array.h>

namespace sail {


// TODO: Implement ProportionateIndexer using this
template <typename T>
class ReduceTree {
public:
  static int left(int i) {return 2*i + 1;}
  static int right(int i) {return 2*i + 2;}
  static int parent(int i) {return (i - 1)/2;}
  static bool isRoot(int i) {return i == 0;}

  ReduceTree(std::function<T(T, T)> reducer,
      Array<T> initialData) :
        _reducer(reducer) {
    int n = initialData.size();
    int l = 0;
    int r = 0;
    while (r - l + 1 < n) {
      l = left(l);
      r = right(r);
    }
    int totalSize = l + n;
    _valid = Array<bool>::fill(r + 1, false);
    _allData = Array<T>(totalSize);
    _leafOffset = l;
    _leaves = _allData.sliceFrom(_leafOffset);
    assert(_leaves.size() == n);
    initialData.copyToSafe(_leaves);
    _valid[0] = true;
    for (int i = 0; i < n; i++) {
      markValid(_leafOffset + i);
    }
    initializeTree(0);
  }

  const T &top() const {
    return _allData[0];
  }

  Array<T> allData() const {
    return _allData;
  }

  void setLeafValue(int index0, const T &value) {
    assert(0 <= index0);
    assert(index0 < _leaves.size());
    setNodeValue(_leafOffset + index0, value);
  }

  void setNodeValue(int index, const T &value) {
    assert(_leafOffset <= index);
    _allData[index] = value;
    while (!isRoot(index)) {
      index = parent(index);
      auto result = _allData[left(index)];
      int r = right(index);
      if (contains(r)) {
        result = _reducer(result, _allData[r]);
      }
      _allData[index] = result;
    }
  }

  T getLeafValue(int index) const {
    return _leaves[index];
  }

  T getNodeValue(int index) const {
    assert(contains(index));
    return _allData[index];
  }

  bool isInner(int i) const {
    return i < _leafOffset;
  }

  bool isLeaf(int i) const {
    return !isInner(i);
  }

  bool contains(int i) const {
    return _valid[i];
  }

  Array<T> leaves() const {
    return _leaves;
  }

  int findNode(const T &value, int index) const {
    if (value < (value - value)/*0*/) {
      return -1;
    }
    if (isLeaf(index)) {
      return index;
    } else {
      auto l = left(index);
      auto r = right(index);
      if (contains(l) && value < getNodeValue(l)) {
        return findNode(value, l);
      } else if (contains(r)) {
        return findNode(value - getNodeValue(l), r);
      }
      return -1;
    }
  }

  int findLeafIndex(const T &value) const {
    auto nodeIndex = findNode(value, 0);
    return nodeIndex == -1? -1 : nodeIndex - _leafOffset;
  }

  T integrate(int toLeaf) const {
    if (_allData.empty()) {
      auto k = T();
      return k - k;
    }
    int nodeIndex = toLeaf + _leafOffset;
    int k = _leaves.size();
    T sum = _allData[0] - _allData[0];
    if (toLeaf < 0) {
      return sum;
    } else if (k <= toLeaf) {
      return _allData[0];
    }

    while (!isRoot(nodeIndex)) {
      int next = parent(nodeIndex);
      if (right(next) == nodeIndex) {
        sum = sum + _allData[left(next)];
      }
      nodeIndex = next;
    }
    return sum;
  }
private:
  std::function<T(T, T)> _reducer;
  Array<T> _allData;
  int _leafOffset;
  Array<T> _leaves;
  Array<bool> _valid;

  void markValid(int i) {
    while (!isRoot(i) && !_valid[i]) {
      _valid[i] = true;
      i = parent(i);
    }
  }

  T initializeTree(int index) {
    if (isInner(index)) {
      auto result = initializeTree(left(index));
      int r = right(index);
      if (contains(r)) {
        auto rightResult = initializeTree(r);
        result = _reducer(result, rightResult);
      }
      _allData[index] = result;
      return result;
    } else {
      return _allData[index];
    }
  }
};

}

#endif /* SERVER_COMMON_REDUCETREE_H_ */
