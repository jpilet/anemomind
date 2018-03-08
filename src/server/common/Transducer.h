/*
 * Transducer.hpp
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TRANSDUCER_H_
#define SERVER_COMMON_TRANSDUCER_H_

#include <iterator>
#include <server/common/ArrayBuilder.h>

namespace sail {

/*
 * Transducers can be used to express transformations of data
 * sequences irrespective how those sequences are represented,
 * which make them reusable in a wide range of contexts.
 * Transducers can be composed into more complex transducers.
 * They do not need to allocate intermediate collections for
 * storing intermediate results, and progressively produce the
 * result as elements arrive.
 *
 * Standard transducers include Map, which is
 * the process of applying a function to every element in a sequence,
 * and Filter, which is the process of rejecting elements that
 * don't satisfy a condition. But we can create a transducer
 * for any sequence algorithm we like and combine them however
 * we like.
 *
 * See also
 * http://vitiy.info/cpp14-how-to-implement-transducers
 * https://github.com/Ableton/atria
 *
 * Very good explanation for transducers in JavaScript:
 * https://tgvashworth.com/2014/08/31/csp-and-transducers.html
 *
 ******* Bastardized version
 *
 * Here is a version which is a bit
 * different from the original formulation, in the
 * sense that we don't need so many template hacks,
 * decltype, etc. It is thus easier to implement and maintain.
 *
 */


template <typename T>
class IntoArray {
public:
  IntoArray(size_t n = 0) : _dst(n) {}

  void add(const T& x) {
    _dst.add(x);
  }

  Array<T> result() {
    return _dst.get();
  }

  void flush() {}
private:
  sail::ArrayBuilder<T> _dst;
};

template <typename F, typename T>
class IntoReduction {
public:
  IntoReduction(F f, T init) : _f(f), _result(init) {}

  template <typename X>
  void add(const X& x) {
    _result = _f(_result, x);
  }

  const T& result() const {
    return _result;
  }

  void flush() {}
private:
  F _f;
  T _result;
};

template <typename T, typename F>
IntoReduction<F, T> intoReduction(F f, T init) {
  return IntoReduction<F, T>(f, init);
}

template <typename A, typename B>
struct CompositeTransducer {
  A a;
  B b;

  template <typename C>
  CompositeTransducer<A, CompositeTransducer<B, C>> operator| (C c) const {
    return {a, {b, c}};
  }

  template <typename T>
  auto apply(T x) -> decltype(a.apply(b.apply(x))) {
    return a.apply(b.apply(x));
  }
};

template <typename F>
class GenericTransducer {
public:
  GenericTransducer(F f) : _f(f) {}

  template <typename Result>
  class Step {
  private:
    Result _result;
  public:

    Step(F f, Result r) : _f(f), _result(r) {}

    template <typename T>
    void add(T x) {
      _f.apply(&_result, x);
    }

    void flush() {
      _f.flush(&_result);
    }

    auto result() -> decltype(_result.result()) {
      return _result.result();
    }
  private:
    F _f;
  };

  template <typename Result>
  Step<Result> apply(Result inner) const {
    return Step<Result>(_f, inner);
  }

  template <typename T>
  CompositeTransducer<GenericTransducer<F>, T> operator| (T x) const {
    return {*this, x};
  }
private:
  F _f;
};

struct trIdentity {
  template <typename Result>
  Result apply(Result x) const {
    return x;
  }

  template <typename T>
  T operator| (T x) const {
    return x;
  }
};

// Just for convenience.
template <typename Stepper>
GenericTransducer<Stepper> genericTransducer(Stepper s) {
  return GenericTransducer<Stepper>(s);
}


struct StatelessStepper {
  template <typename Result> void flush(Result* r) {
    r->flush();
  }
};

//// Helper types
template <typename F>
struct MapStepper : StatelessStepper {
  F f;
  MapStepper(F fn) : f(fn) {}

  template <typename R, typename T>
  void apply(R* dst, T x) {
    dst->add(f(x));
  }
};

template <typename F>
struct FilterStepper : public StatelessStepper {
  F f;

  FilterStepper(F fn) : f(fn) {}

  template <typename R, typename T>
  void apply(R* dst, T x) {
    if (f(x)) {
      dst->add(x);
    }
  }
};

// For merging two collections, one of them
// having its iterators stored in this stepper.
template <typename Comparator, typename Iterator>
struct MergeStepper {
  Comparator comp;
  Iterator begin;
  Iterator end;
  MergeStepper(Comparator c, Iterator b, Iterator e)
    : comp(c), begin(b), end(e) {}

  template <typename R, typename T>
  void apply(R* dst, T x) {
    while (begin != end && comp(*begin, x)) {
      dst->add(*begin);
      begin++;
    }
    dst->add(x);
  }

  template <typename R>
  void flush(R* dst) {
    for (; begin != end; begin++) {
      dst->add(*begin);
    }
    dst->flush();
  }
};

// Common transducer types

template <typename F>
GenericTransducer<MapStepper<F>> trMap(F f) {
  return genericTransducer(MapStepper<F>{f});
}

template <typename F>
GenericTransducer<FilterStepper<F>> trFilter(F f) {
  return genericTransducer(FilterStepper<F>{f});
}

// Used to merge-in a range of elements with the incoming elements
template <
  typename Iterator,
  typename Comp = std::less<typename Iterator::value_type>>
GenericTransducer<MergeStepper<Comp, Iterator>> trMerge(
    Iterator b, Iterator e, Comp c = Comp()) {
  return genericTransducer(MergeStepper<Comp, Iterator>(c, b, e));
}

/**
 * The main function, that takes an iterable source collection
 * and transduces it into a result using the transducer tr.
 */
template <typename Coll, typename Transducer, typename Result>
auto transduce(const Coll& coll, Transducer tr, Result r0)
  -> decltype(tr.apply(r0).result()) {
  auto r = tr.apply(r0);
  for (const auto& x: coll) {
     r.add(x);
  }
  r.flush();
  return r.result();
}

}




#endif /* SERVER_COMMON_TRANSDUCER_H_ */
