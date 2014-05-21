/*
 *  Created on: 2014-05-21
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ARRAYSTORAGE_H_
#define ARRAYSTORAGE_H_

#include <memory>
#include <vector>
#include <cassert>

namespace sail {

template <typename T>
class ArrayStorage {
 private:
  typedef std::vector<T> Vector;
  typedef std::shared_ptr<Vector> VectorPtr;
  typedef ArrayStorage<T> ThisType;
 public:
  ArrayStorage() {}

  ArrayStorage(int s) : _data(new Vector(s)) {}

  ArrayStorage(const VectorPtr &ptr) : _data(ptr) {}

  ThisType dup() const {return ThisType(_data);}

  bool allocated() const {return bool(_data);}

  int size() const {
    assert(allocated());
    return _data->size();
  }

  T *ptr() {
    assert(allocated());
    Vector &v = *_data;
    return &(v[0]);
  }

  const Vector &vector() const {
    assert(allocated());
    return *_data;
  }
 private:
  VectorPtr _data;
};

}

#endif /* ARRAYSTORAGE_H_ */
