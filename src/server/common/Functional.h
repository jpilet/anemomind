/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 *
 *  Functions that operate on collection type objects.
 *  Objects that are indexable with the [] operator and has a .size() method,
 *  such as std::vector, sail::Array, sail::Spani, etc.
 */

#ifndef SERVER_COMMON_FUNCTIONAL_H_
#define SERVER_COMMON_FUNCTIONAL_H_

#include <server/common/Array.h>
#include <server/common/ArrayBuilder.h>
#include <utility>
#include <server/common/logging.h>
#include <cassert>

namespace sail {

template <typename Array>
class ArrayIterator {
 private:
  Array _array;
  int _index;
 public:
  typedef decltype(_array[0]) ElementType;

  ArrayIterator(Array array, int index) :
    _array(array), _index(index) {}

  ElementType operator*() const {
    return _array[_index];
  }

  void operator++() {
    _index++;
  }

  bool operator== (const ArrayIterator &other) const {
    return _index == other._index;
  }

  bool operator!= (const ArrayIterator &other) const {
    return !((*this) == other);
  }
};

// To make a reference into a value.
template <typename T>
constexpr T copyOf(T x) {
  return x;
}

// Evaluate an abstract collection to an array
template <typename Collection>
auto toArray(const Collection &c) -> Array<decltype(c[0])> {
  Array<decltype(c[0])> dst(c.size());
  for (int i = 0; i < c.size(); i++) {
    dst[i] = c[i];
  }
  return dst;
}

inline int getCommonArraySize() {
  return -1;
}

inline bool compatibleArraySizes(int firstSize, int remainingSize) {
  return firstSize == remainingSize || remainingSize == -1;
}

template <typename First, typename... Rest>
int getCommonArraySize(First f, Rest... rest) {
  assert(compatibleArraySizes(f.size(), getCommonArraySize(rest...)));
  return f.size();
}


// Needs to be visible by vmap just below, used to infer return type.
template <typename ResultType>
class Mapped;

// Needs to be visible by map just below, used to infer return type.
template <typename Function, typename... Collections>
auto vmap(Function f, Collections... colls) -> Mapped<decltype(f((colls[0])...))>;

// sail::map needs to be visible to Mapped::map, that uses it
// to infer its return type.
template <typename Function, typename Collection>
auto map(Collection X, Function f) -> decltype(vmap(f, X));

template <typename Function, typename Collection>
auto reduce(Collection X, Function f) -> decltype(f(X[0], X[1]));

template <typename ResultType>
class Mapped {
 public:
  typedef Mapped<ResultType> ThisType;

  Mapped(Array<ResultType> src) : _size(src.size()) {
    _f = [=](int i) {return src[i];};
  }

  Mapped(std::function<ResultType(int)> f, int size) : _f(f), _size(size) {}

  int size() const {
    return _size;
  }

  ArrayIterator<ThisType> begin() const {
    return ArrayIterator<ThisType>(*this, 0);
  }

  ArrayIterator<ThisType> end() const {
    return ArrayIterator<ThisType>(*this, size());
  }

  ResultType operator[] (int index) const {
    return _f(index);
  }

  auto toArray() const -> decltype(sail::toArray(*this)) {
    return sail::toArray(*this);
  }

  template <typename Function>
  auto map(Function f) const -> Mapped<decltype(f(std::declval<ResultType>()))> {
    return sail::map(*this, f);
  }

  template <typename Function>
  auto reduce(Function f) const -> decltype(f((*this)[0], (*this)[1])) {
    return sail::reduce(*this, f);
  }

  operator Array<ResultType>() const {
    return toArray();
  }

  void putInArray(ResultType *dst) const {
    for (int i = 0; i < _size; i++) {
      dst[i] = _f(i);
    }
  }
 private:
  int _size;
  std::function<ResultType(int)> _f;
};

// Variadic map.
template <typename Function, typename... Collections>
auto vmap(Function f, Collections... colls) -> Mapped<decltype(f((colls[0])...))> {
  auto mapper = [=](int index) {
    return f((colls[index])...);
  };
  typedef decltype(mapper(0)) ResultType;
  return Mapped<ResultType>(mapper, getCommonArraySize(colls...));
}

template <typename Function, typename Collection>
auto map(Collection X, Function f) -> decltype(vmap(f, X)) {
  return vmap(f, X);
}

template <typename Function, typename CollectionA, typename CollectionB>
auto map(CollectionA X, CollectionB Y, Function f) -> decltype(vmap(f, X, Y)) {
  return vmap(f, X, Y);
}

template <typename DataCollection, typename IndexCollection>
auto subsetByIndex(DataCollection data, IndexCollection indices)
    -> Mapped<decltype(copyOf(data[0]))> {
  return map(indices, [=](int index) {return data[index]; });
}

// Filter
template <typename Function, typename Collection>
auto filter(Collection X, Function f) -> Array<decltype(X[0])> {
  int n = X.size();
  int counter = 0;
  ArrayBuilder<decltype(X[0])> Y(n);
  for (int i = 0; i < n; i++) {
    if (f(X[i])) {
      Y.add(X[i]);
    }
  }
  return Y.get();
}

// Reduce
template <typename Function, typename Collection>
auto reduce(Collection X, Function f) -> decltype(f(X[0], X[1])) {
  int n = X.size();
  assert(0 < n);
  auto acc = X[0];
  for (int i = 1; i < n; i++) {
    acc = f(acc, X[i]);
  }
  return acc;
}

// Reduce, given an initial value
template <typename Function, typename Collection, typename Init>
auto reduce(Init init, Collection X, Function f) -> decltype(f(init, X[0])) {
  auto acc = init;
  int n = X.size();
  for (int i = 0; i < n; i++) {
    acc = f(acc, X[i]);
  }
  return acc;
}

template <typename ArrayOfArrays>
auto concat(ArrayOfArrays arrayOfArrays) -> Array<decltype(copyOf(std::declval<ArrayOfArrays>()[0][0]))> {
  typedef decltype(copyOf(std::declval<ArrayOfArrays>()[0][0])) ElementType;
  int elementCount = 0;
  for (auto array: arrayOfArrays) {
    elementCount += array.size();
  }
  ArrayBuilder<ElementType> dst(elementCount);
  for (auto array: arrayOfArrays) {
    for (auto e: array) {
      dst.add(e);
    }
  }
  assert(dst.size() == elementCount);
  return dst.get();
}

}

#undef ADD_METHODS_FOR_MAPPED
#endif /* SERVER_COMMON_FUNCTIONAL_H_ */
