/*
 *  Created on: 2014-05-21
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ARRAYSTORAGE_H_
#define ARRAYSTORAGE_H_

#include <memory>
#include <vector>
#include <cassert>

namespace Eigen {
  template<typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
  class Matrix;

  template <typename T>
  class aligned_allocator;
};

namespace sail {

namespace ArrayStorageInternal {
  /*
   * Hack used to bypass the specialization for std::vector<bool>
   *
   * See http://www.cplusplus.com/reference/vector/vector-bool/
   *
   * While this space optimization in the C++ library may seem like a good idea, it is not practical
   * when ArrayStorage<T> is used together with Array<T> that holds a pointer T *_data
   * (with T=bool) to memory that is either from the stack or from ArrayStorage<T>. Therefore,
   * we have to bypass this specialization somehow if we want to use std::vector.
   */
  template <typename T>
  class ElementType {
   public:
    typedef T InternalType;
  };

  template <>
  class ElementType<bool> {
   public:
    typedef unsigned char InternalType;
    static_assert(sizeof(InternalType) == sizeof(bool), "Bad size");
  };

  template <typename T>
  struct VectorType {
    typedef std::vector<T> type;
  };

  template<typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
  struct VectorType<Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>> {
    typedef Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> value_type;
    typedef std::vector<value_type, Eigen::aligned_allocator<value_type>> type;
  };

  template <typename T>
  using AlignedVector = typename VectorType<T>::type;
}



template <typename T>
class ArrayStorage {
 private:
  typedef ArrayStorage<T> ThisType;
 public:
  typedef ArrayStorageInternal::AlignedVector<
      typename ArrayStorageInternal::ElementType<T>::InternalType> Vector;
  typedef std::shared_ptr<Vector> VectorPtr;

  ArrayStorage() {}

  ArrayStorage(int s) : _data(new Vector(s)) {}

  ArrayStorage(const VectorPtr &ptr) : _data(ptr) {}

  ThisType dup() const {return ThisType(_data(new Vector(vector())));}

  bool allocated() const {return bool(_data);}

  int size() const {
    assert(allocated());
    return _data->size();
  }

  T *ptr() {
    assert(allocated());
    Vector &v = *_data;
    return (T *)(&(v[0]));
  }

  const Vector &vector() const {
    assert(allocated());
    return *_data;
  }

  bool operator== (const ThisType &other) const {
    return _data == other._data;
  }
 private:
  VectorPtr _data;
};

}

#endif /* ARRAYSTORAGE_H_ */
