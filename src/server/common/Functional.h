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

template <typename First, typename... Rest>
int getFirstSize(First f, Rest... rest) {
  return f.size();
}


template <typename ResultType>
class Mapped;
template <typename Function, typename... Collections>
auto vmap(Function f, Collections... colls) -> Mapped<decltype(f((colls[0])...))>;
template <typename Function, typename Collection>
auto map(Collection X, Function f) -> decltype(vmap(f, X));

template <typename ResultType>
class Mapped {
 public:
  Mapped(std::function<ResultType(int)> f, int size) : _f(f), _size(size) {}

  int size() const {
    return _size;
  }

  ResultType operator[] (int index) const {
    return _f(index);
  }

  auto toArray() const -> decltype(sail::toArray(*this)) {
    return sail::toArray(*this);
  }

  template <typename Function>
  auto map(Function f) const -> decltype(sail::map(*this, f)) {
    return sail::map(*this, f);
  }
 private:
  int _size;
  std::function<ResultType(int)> _f;
};

template <typename Function, typename... Collections>
auto vmap(Function f, Collections... colls) -> Mapped<decltype(f((colls[0])...))> {
  auto indexedResults = [=](int index) {
    return f((colls[index])...);
  };
  typedef decltype(indexedResults(0)) ResultType;
  return Mapped<ResultType>(indexedResults, getFirstSize(colls...));
}


template <typename Function, typename Collection>
auto map(Collection X, Function f) -> decltype(vmap(f, X)) {
  return vmap(f, X);
}

template <typename Function, typename CollectionA, typename CollectionB>
auto map(CollectionA X, CollectionB Y, Function f) -> decltype(vmap(f, X, Y)) {
  return vmap(f, X, Y);
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
