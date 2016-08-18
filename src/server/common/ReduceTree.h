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
    _allData = Array<T>(totalSize);
    _leafOffset = l;
    _leaves = _allData.sliceFrom(_leafOffset);
    assert(_leaves.size() == n);
    initialData.copyToSafe(_leaves);
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
    return _allData[index];
  }

  bool isInner(int i) const {
    return i < _leafOffset;
  }

  bool isLeaf(int i) const {
    return !isInner(i);
  }

  bool contains(int i) const {
    return i < _allData.size();
  }

  Array<T> leaves() const {
    return _leaves;
  }
private:
  std::function<T(T, T)> _reducer;
  Array<T> _allData;
  int _leafOffset;
  Array<T> _leaves;

  T initializeTree(int index) {

    *((unsigned long *)nullptr) = 0xDEADBEEF; // FIx this,

    if (isInner(index)) {
      auto result = initializeTree(left(index));
      int r = right(index);
      if (contains(r)) {
        auto rightResult = initializeTree(r);
        result = _reducer(result, rightResult);
      }

      std::cout << "Inner node " << index << " result: " << result << std::endl;
      std::cout << "  leaf offset: " << _leafOffset << std::endl;
      std::cout << "  Left index: " << left(index) << std::endl;
      std::cout << "  Right index: " << right(index) << std::endl;
      std::cout << "  Data size: " << _allData.size() << std::endl;

      _allData[index] = result;
      return result;
    } else {
      return _allData[index];
    }
  }

};

}



#endif /* SERVER_COMMON_REDUCETREE_H_ */
