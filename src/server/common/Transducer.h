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
 * See also
 * http://vitiy.info/cpp14-how-to-implement-transducers
 * https://github.com/Ableton/atria
 *
 * Very good explanation for transducers in JavaScript:
 * https://tgvashworth.com/2014/08/31/csp-and-transducers.html
 *
 */

// This is a Step that doesn't change the result
template <typename R, typename X>
struct NoStep {
  typedef R result_type;
  typedef X input_type;
  R step(R r, X x) const {return r;}
  R flush(R r) const {return r;}
};



// This is a type to denote that the type is undefined
struct UndefinedType {};

// This is used as a placeholder in transducers
typedef NoStep<UndefinedType, UndefinedType> UndefinedStep;

/////////////////////////////////////////////////////////////////////
/////////////// Transducer base type, and composition
///////////////
template <typename ThisType>
class Transducer;

template <typename A, typename B>
class ComposedTransducer : public Transducer<ComposedTransducer<A, B>> {
private:
  A _a;
  B _b;
public:
  ComposedTransducer(const A& a, const B& b) : _a(a), _b(b) {}

  template <typename Step>
  auto apply(Step s) -> decltype(_a.apply(_b.apply(s))) {
    return _a.apply(_b.apply(s));
  }
};

// Base class. Decorates the transducer with an | operator.
template <typename ThisType>
class Transducer {
public:
  template <typename Other>
  ComposedTransducer<ThisType, Other> operator| (const Other& y) const {
    return ComposedTransducer<ThisType, Other>(
        *reinterpret_cast<const ThisType*>(this), y);
  }
};




/*
 * Implementation detail:
 *
 * Here follows some standard transducer types, e.g. Map and Filter.
 *
 * Each type is implemented using a single template class that
 * works both as a Step function (with methods 'step' and 'flush')
 * and as a Transducer (with a method 'apply').
 * In the case we use it as a transducer, we don't know the type of
 * the Step, so we can just use 'UndefinedStep' type as template
 * parameter.
 *
 * The reason for this is that it leads to a more compact implementation
 * with less code duplication (and therefore also more maintainable).
 *
 */

/////////////////////////////////////////////////////////////////////
/////////////////////// Mapping transducer

template <typename F, typename Step>
class Map : public Step, public Transducer<Map<F, Step>> {
public:
  Map(const F& f, const Step& s = Step()) : _f(f), Step(s) {}

  typedef FirstType<CleanFunctionArgTypes<F>> input_type;
  typedef typename Step::result_type result_type;

  // Use it as a step function
  result_type step(result_type y, input_type x) {
    return Step::step(y, _f(x));
  }

  // Use it as a transducer
  template <typename S>
  Map<F, S> apply(S s) const {
    return Map<F, S>(_f, s);
  }
private:
  F _f;
};

template <typename F>
Map<F, UndefinedStep> trMap(const F& f) {
  return Map<F, UndefinedStep>(f);
}

///////////////////////////////////////////////////////////////////
/// trVisit: Like map, but only for side effects. The input argument
///          is returned.

// Helper struct
template <typename F>
struct Visitor {
  typedef FirstType<CleanFunctionArgTypes<F>> input_type;
  Visitor(F f) : _f(f) {}

  input_type operator()(input_type x) const {
    _f(x);
    return x;
  }
private:
  F _f;
};

template <typename F>
Map<Visitor<F>, UndefinedStep> trVisit(F f) {
  return Map<Visitor<F>, UndefinedStep>(Visitor<F>(f));
}


/////////////////////////////////////////////////////////////////////
/////////////////////// Filtering transducer
template <typename F, typename Step>
class Filter : public Step, public Transducer<Filter<F, Step>> {
public:
  Filter(const F& f, const Step& s = Step()) : _f(f), Step(s) {}

  typedef typename Step::input_type input_type;
  typedef typename Step::result_type result_type;

  // Use it as a step function
  result_type step(result_type y, input_type x) {
    return _f(x)? Step::step(y, x) : y;
  }

  // Use it as a transducer
  template <typename S>
  Filter<F, S> apply(S s) const {
    return Filter<F, S>(_f, s);
  }
private:
  F _f;
};

template <typename F>
Filter<F, UndefinedStep> trFilter(const F& f) {
  return Filter<F, UndefinedStep>(f);
}

/////////////////////////////////////////////////////////////////////
/////////////////////// Concatenating transducer
template <typename Coll, typename Step>
struct Cat : public Step, public Transducer<Cat<Coll, Step>> {
  Cat(const Step& s = Step()) : Step(s) {}

  typedef Coll input_type;
  typedef typename Step::result_type result_type;

  result_type step(result_type y, const input_type& X) {
    auto dst = y;
    for (auto x: X) {
      dst = Step::step(dst, x);
    }
    return dst;
  }

  template <typename S>
  Cat<Coll, S> apply(S s) const {
    return Cat<Coll, S>(s);
  }
};

template <typename Coll>
Cat<Coll, UndefinedStep> trCat() {
  return Cat<Coll, UndefinedStep>();
}



///////// Basic step functions
template <typename T>
struct AddStep : NoStep<T, T> {
  T step(T x, T y) const {return x + y;}
};

template <typename T>
struct CountStep : NoStep<T, T> {
  T step(T x, T) const {return x + 1;}
};

template <typename Iterator>
using IteratorStepBase = NoStep<Iterator,
    typename IteratorInputType<Iterator>::type>;

template <typename Iterator>
struct IteratorStep : public IteratorStepBase<Iterator> {
  typedef IteratorStepBase<Iterator> Base;

  typename Base::result_type step(
      typename Base::result_type y,
      typename Base::input_type x) const {
    *y = x;
    return y;
  }
};

/// Step function for adding something at the end of
/// a collection. The accumulated type is the iterator,
/// and not the collection itself.
template <typename Iterator>
IteratorStep<Iterator> iteratorStep(Iterator i) {
  return IteratorStep<Iterator>();
}



// This reduces over a collection
// (anything that exhibits iterators begin() and end() as in the STL)
// using a step function, and flushes at the end.
template <typename Step, typename Dst, typename Coll>
Dst reduce(Step step, Dst init, const Coll& coll) {
  Dst acc = init;
  for (const auto& x: coll) {
    acc = step.step(acc, x);
  }
  return step.flush(acc);
}

// This is a convenience function for reducing, when the accumulated
// type is a collection.
template <typename T, typename Dst, typename Src>
void transduceIntoColl(T transducer, Dst* dst, const Src& src) {
  auto i = std::inserter(*dst, dst->end());
  reduce(transducer.apply(iteratorStep(i)), i, src);
}

}




#endif /* SERVER_COMMON_TRANSDUCER_H_ */
