/*
 * Transducer.hpp
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TRANSDUCER_H_
#define SERVER_COMMON_TRANSDUCER_H_

#include <server/common/traits.h>
#include <iterator>

namespace sail {

/*
 * Transducers can be used to express and compose transformations of
 * data sequences irrespective how those sequences are represented,
 * which means they can be used in many different contexts.
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

  Step base(StepFunction step) {
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

///
template <typename Dst, typename X, typename Coll>
Dst reduce(Step<Dst, X> step, Dst init, const Coll& coll) {
  Dst acc = init;
  for (X x: coll) {
    acc = step.step(acc, x);
  }
  return step.flush(acc);
}

/// Standard step functions
template <typename Iterator>
Step<Iterator, typename std::iterator_traits<Iterator>::value_type>
  iteratorStep(Iterator i) {
  typedef typename std::iterator_traits<Iterator>::value_type X;
  return Step<Iterator, X>::base(
    [](Iterator i, X x) {*i = x; return ++i;}
  );
}


}




#endif /* SERVER_COMMON_TRANSDUCER_H_ */
