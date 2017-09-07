/*
 * Transducer.hpp
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TRANSDUCER_H_
#define SERVER_COMMON_TRANSDUCER_H_

#include <iterator>
#include <server/common/traits.h>

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
 */

/*
 * A step represents a function that
 * takes a result which is being accumulated and
 * a new value that should be incorporated, and returns
 * the accumulated value. It is fine to change the accumulated
 * value.
 *
 * A stateful transducer may have to flush its state. That is
 * what the flush function is for. If the transducer is stateless,
 * no need to implement a custom flush, it can just take the flush
 * function of the state it transforms.
 */
template <typename AccumulateType, typename InputType>
struct Step {
  typedef std::function<AccumulateType(AccumulateType, InputType)> StepFunction;
  typedef std::function<AccumulateType(AccumulateType)> FlushFunction;
  StepFunction step;
  FlushFunction flush;

  static Step base(StepFunction step) {
    return {step, [](AccumulateType x) {return x;}};
  }
};

/*
 * A mapping transducer will tweak a step function so that
 * the value is mapped before the step is applied.
 */
template <typename F>
class Map {
public:
  Map(const F& f) : _f(f) {}

  typedef FunctionResultType<F> Y;
  typedef FunctionArgTypes<F> ArgTypes;
  typedef FirstType<ArgTypes> X;
  static_assert(Arity<F>::value == 1, "");

  template <typename R>
  Step<R, X> operator() (const Step<R, Y>& s) const {
    auto f = _f;
    return {
      [f, s](R r, X x) {return s.step(r, f(x));},
      s.flush, // no custom flushing.
    };
  }
private:
  F _f;
};
template <typename F> Map<F> map(const F& f) {
  return Map<F>(f);
};

/*
 * A filtering transducer will tweak the step function so that
 * it will only accept elements that satisfy a condition.
 */
template <typename F>
struct Filter {
public:
  Filter(const F& f) : _f(f) {}

  typedef FunctionArgTypes<F> ArgTypes;
  typedef FirstType<ArgTypes> X;
  static_assert(std::is_same<FunctionResultType<F>, bool>::value, "");
  static_assert(Arity<F>::value == 1, "");

  template <typename R>
  Step<R, X> operator() (const Step<R, X>& s) const {
    auto f = _f;
    return {
      [f, s](R r, X x) {
        return f(x)? s.step(r, x) : r;
      },
      s.flush // no custom flushing.
    };
  }
private:
  F _f;
};
template <typename F> Filter<F> filter(const F& f) {
  return Filter<F>(f);
};

template <typename X, typename Y>
class Compose2 {
public:
  Compose2(X x, Y y) : _x(x), _y(y) {}

  template <typename T>
  auto operator()(const T& x) -> decltype(_x(_y(x))) {
    return _x(_y(x));
  }
private:
  X _x;
  Y _y;
};



// Composing two transducers results in a new transducer
template <typename A, typename B>
class ComposeTransducers2 {
public:
  ComposeTransducers2(const A& a, const B& b) : _a(a), _b(b) {}

  template <typename S>
  auto operator()(const S& b)
    -> decltype(std::declval<A>()(
        std::declval<B>()(b))) {
    return _a(_b(b));
  }
private:
  A _a;
  B _b;
};

// Used to compose many transducers into a pipeline.
template <typename... T> class ComposeTransducers {};

template <typename X, typename... Y>
class ComposeTransducers<X, Y...> {
public:
  typedef ComposeTransducers2<X, ComposeTransducers<Y...>> Inner;

  ComposeTransducers(X x, Y... y)
    : _inner(x, ComposeTransducers<Y...>(y...)) {}

  template <typename S>
  auto operator() (const S& s) -> decltype(
      std::declval<Inner>()(std::declval<S>())) {
    return _inner(s);
  }
private:
  Inner _inner;
};


template <typename X, typename Y>
class ComposeTransducers<X, Y> : public ComposeTransducers2<X, Y> {
public:
  using ComposeTransducers2<X, Y>::ComposeTransducers2;
};

template <typename ... T>
ComposeTransducers<T...> composeTransducers(T... x) {
  return ComposeTransducers<T...>(x...);
}

// This reduces over a collection
// (anything that exhibits iterators begin() and end() as in the STL)
// using a step function, and flushes at the end.
template <typename Dst, typename X, typename Coll>
Dst reduce(Step<Dst, X> step, Dst init, const Coll& coll) {
  Dst acc = init;
  for (const X& x: coll) {
    acc = step.step(acc, x);
  }
  return step.flush(acc);
}

/// Step function for adding something at the end of
/// a collection. The accumulated type is the iterator,
/// and not the collection itself.
template <typename Iterator>
Step<Iterator, typename IteratorInputType<Iterator>::type>
  iteratorStep(Iterator i) {
  typedef typename IteratorInputType<Iterator>::type X;
  return Step<Iterator, X>::base(
    [](Iterator i, X x) {*i = x; return ++i;}
  );
}

// This is a convenience function for reducing, when the accumulated
// type is a collection.
template <typename T, typename Dst, typename Src>
void transduceIntoColl(T transducer, Dst* dst, const Src& src) {
  auto i = std::inserter(*dst, dst->end());
  reduce(transducer(iteratorStep(i)), i, src);
}

}




#endif /* SERVER_COMMON_TRANSDUCER_H_ */
