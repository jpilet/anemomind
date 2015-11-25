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

template <typename Function, typename Collection>
class Mapped;


template <typename Function, typename Collection>
Mapped<Function, Collection> map(Collection X, Function f);


template <typename TType, typename RType>
class MappedBase {
 public:
  typedef TType ThisType;
  typedef RType ResultType;

  Array<ResultType> toArray() const {
    return sail::toArray(*static_cast<const ThisType*>(this));
  }

  operator Array<ResultType>() const {
    return toArray();
  }

  ArrayIterator<ThisType> begin() const {
    return ArrayIterator<ThisType>(*static_cast<const ThisType*>(this), 0);
  }

  ArrayIterator<ThisType> end() const {
    return ArrayIterator<ThisType>(*static_cast<const ThisType*>(this), 0);
  }

  ResultType operator[](int index) const {
    return (*static_cast<const ThisType*>(this))[index];
  }

  template <typename AnotherFunctionType> \
  auto map(AnotherFunctionType f) -> decltype(sail::map(*static_cast<const ThisType*>(this), f)) {
    return sail::map(*static_cast<const ThisType*>(this), f);
  }
};

template <typename Function, typename Collection>
struct UnaryMapping {
  typedef decltype(std::declval<Function>()(std::declval<Collection>()[0])) ResultType;
};


template <typename Function, typename Collection>
class Mapped : public MappedBase<Mapped<Function, Collection>,
  typename UnaryMapping<Function, Collection>::ResultType> {
 public:
  Mapped(Function f, const Collection &A) :
    _f(f), _A(A) {
  }

  int size() const {
    return _A.size();
  }

  // The ResultType is also typedef'ed in
  // MappedBase, but doesn't seem to be visible here
  // for some reason.
  typename UnaryMapping<Function, Collection>::ResultType
    operator[] (int index) const {
    return _f(_A[index]);
  }
 private:
  Function _f;
  Collection _A;
};

// Map
// Returns an array like object, but not an Array.
// This is to avoid silly allocations of temporary arrays
// if we chain multiple calls to map, or if we reduce
// just after calling map. If we want an Array, call
// toArray implemented at the bottom of this file.
template <typename Function, typename Collection>
Mapped<Function, Collection> map(Collection X, Function f) {
  return Mapped<Function, Collection>(f, X);
}

template <typename Function, typename CollectionA, typename CollectionB>
struct BinaryMapping {
  typedef decltype(std::declval<Function>()(std::declval<CollectionA>()[0],
        std::declval<CollectionB>()[0])) ResultType;
};

// TODO: Use parameter packs (variadic templates) to support any number of collections.
// But this feature doesn't seem to be completely implemented in some compilers :-(
// See http://stackoverflow.com/a/22569362
template <typename Function, typename CollectionA, typename CollectionB>
class Mapped2 : public MappedBase<Mapped2<Function, CollectionA, CollectionB>,
  typename BinaryMapping<Function, CollectionA, CollectionB>::ResultType> {
 public:
  Mapped2(Function f, const CollectionA &A, const CollectionB &B) :
    _f(f), _A(A), _B(B) {
    assert(A.size() == B.size());
  }

  int size() const {
    return _A.size();
  }

  typename BinaryMapping<Function, CollectionA, CollectionB>::ResultType
    operator[] (int index) const {
    return _f(_A[index], _B[index]);
  }
 private:
  Function _f;
  CollectionA _A;
  CollectionB _B;
};

template <typename Function, typename CollectionA, typename CollectionB>
Mapped2<Function, CollectionA, CollectionB> map(
    const CollectionA &X, const CollectionB &Y, Function f) {
  return Mapped2<Function, CollectionA, CollectionB>(f, X, Y);
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
