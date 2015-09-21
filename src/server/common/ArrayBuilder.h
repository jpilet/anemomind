/*
 *  Created on: 2014-04-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Use ArrayBuilder to add elements to an array when the final number of
 *  elements is not known.
 */

#ifndef ARRAYBUILDER_H_
#define ARRAYBUILDER_H_

#include <server/common/Array.h>

namespace sail {

template <typename T>
class ArrayBuilder {
 private:
  typedef typename ArrayStorage<T>::Vector Vector;
  typedef typename ArrayStorage<T>::VectorPtr VectorPtr;
 public:
  ArrayBuilder(int expectedMaxCount = 1) {
    _data = VectorPtr(new Vector());
    _data->reserve(expectedMaxCount);
    assert(bool(_data));
  }

  void add(const T &x) {
    assert(bool(_data));
    _data->push_back(x);
  }

  void add(const Array<T> &X) {
    for (auto x : X) {
      add(x);
    }
  }

  void addIf(const Array<T> &X, std::function<bool(const T&)> condition) {
    for (auto x : X) {
      if (condition(x)) {
        add(x);
      }
    }
  }

  Array<T> get() {
    return Array<T>(ArrayStorage<T>(VectorPtr(new Vector(*_data))));
  }
  T &last() {return _data->back();}
  bool empty() {return _data->empty();}
  int size() const {
    return _data->size();
  }
 private:
   VectorPtr _data;
};

}

#endif /* ARRAYBUILDER_H_ */
