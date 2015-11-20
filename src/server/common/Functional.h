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

namespace sail {

template <typename T>
struct AnyValue {
 static T x;
};

// Evaluate an abstract collection to an array
template <typename Collection>
auto toArray(const Collection &c) -> Array<decltype(c[0])> {
  int n = c.size();
  Array<decltype(c[0])> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = c[i];
  }
  return dst;
}

template <typename Function, typename Collection>
class Mapped {
 public:
  typedef decltype(AnyValue<Function>::x(AnyValue<Collection>::x[0])) ResultType;

  Mapped(Function f, const Collection &A) :
    _f(f), _A(A) {
  }

  int size() const {
    return _A.size();
  }

  ResultType operator[] (int index) const {
    return _f(_A[index]);
  }

  Array<ResultType> toArray() const {
    return sail::toArray(*this);
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
Mapped<Function, Collection> map(Function f, Collection X) {
  return Mapped<Function, Collection>(f, X);
}

// TODO: Use parameter packs (variadic templates) to support any number of collections.
// But this feature doesn't seem to be completely implemented in some compilers :-(
// See http://stackoverflow.com/a/22569362
template <typename Function, typename CollectionA, typename CollectionB>
class Mapped2 {
 public:
  typedef decltype(AnyValue<Function>::x(AnyValue<CollectionA>::x[0],
      AnyValue<CollectionB>::x[0])) ResultType;

  Mapped2(Function f, const CollectionA &A, const CollectionB &B) :
    _f(f), _A(A), _B(B) {
    assert(A.size() == B.size());
  }

  int size() const {
    return _A.size();
  }

  ResultType operator[] (int index) const {
    return _f(_A[index], _B[index]);
  }

  Array<ResultType> toArray() const {
    return sail::toArray(*this);
  }
 private:
  Function _f;
  CollectionA _A;
  CollectionB _B;
};

template <typename Function, typename CollectionA, typename CollectionB>
Mapped2<Function, CollectionA, CollectionB> map(
    Function f, const CollectionA &X, const CollectionB &Y) {
  return Mapped2<Function, CollectionA, CollectionB>(f, X, Y);
}


// Filter
template <typename Function, typename Collection>
auto filter(Function f, Collection X) -> Array<decltype(X[0])> {
  int n = X.size();
  int counter = 0;
  Array<decltype(X[0])> Y(n);
  for (int i = 0; i < n; i++) {
    if (f(X[i])) {
      Y[counter] = X[i];
      counter++;
    }
  }
  return Y.sliceTo(counter);
}

// Reduce
template <typename Function, typename Collection>
auto reduce(Function f, Collection X) -> decltype(f(X[0], X[1])) {
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
auto reduce(Function f, Collection X, Init init) -> decltype(f(init, X[0])) {
  auto acc = init;
  int n = X.size();
  for (int i = 0; i < n; i++) {
    acc = f(acc, X[i]);
  }
  return acc;
}

}

#endif /* SERVER_COMMON_FUNCTIONAL_H_ */
