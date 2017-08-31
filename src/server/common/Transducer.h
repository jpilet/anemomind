/*
 * Transducer.h
 *
 *  Created on: 28 Jul 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TRANSDUCER_H_
#define SERVER_COMMON_TRANSDUCER_H_

#include <server/common/traits.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Array.h>

/*
 * NOTE:
 *
 * There are also transducer implementations here
 *
 *   https://github.com/Ableton/atria
 *
 * and here
 *
 *   https://github.com/kirkshoop/transducer
 *
 * Early termination is not implemented.
 *
 */

namespace sail {

// Inherit from this one to get the typedefs needed.
template <typename R, typename X>
class TransducerStep {
public:
  typedef R ResultType;
  typedef X InputType;
};

template <typename Iterator, typename X>
class IteratorStep: public TransducerStep<
  Iterator, typename std::iterator_traits<Iterator>::value_type> {
public:
  Iterator complete(Iterator i) {return i;}

  Iterator step(Iterator i, X x) {
    *i = x;
    return ++i;
  }
};

template <typename X>
class CountingStep : public TransducerStep<int, X> {
public:
  int complete(int n) {return n;}
  int step(int n, X x) {return n+1;}
};

// For convenience
template <typename Iterator>
IteratorStep<Iterator,
  typename IteratorInputType<Iterator>::type> iteratorStep(Iterator) {
  return IteratorStep<Iterator,
      typename IteratorInputType<Iterator>::type>();
}

/*
 * A complete step function also has the methods with signatures
 *
 * R complete(R);
 * R step(R, X);
 *
 * NOTE: R should not be const, it can actually be mutable.
 *
 */

// A Transducer is any class with a method
//
// template <typename S0>
// S1 apply(S0);
//
// That takes a step function S0 as input and
// returns a tweaked step function as output.

/////////////////////////////// Some standard transducers
template <typename Y, typename X>
class Map {
public:
  Map(const std::function<Y(X)>& f) : _f(f) {}

  template <typename S>
  class Step : public TransducerStep<typename S::ResultType, X> {
  public:
    Step(const std::function<Y(X)>& f, S s) : _f(f), _s(s) {}

    typedef typename S::ResultType R;

    R step(R r, X x) {
      return _s.step(r, _f(x));
    }

    R complete(R r) {
      return _s.complete(r);
    }
  private:
    std::function<Y(X)> _f;
    S _s;
  };

  template <typename S>
  Step<S> apply(const S& s) {
    return Step<S>(_f, s);
  }
private:
  std::function<Y(X)> _f;
};

template <typename Y, typename X>
Map<Y, X> map(const std::function<Y(X)>& f) {
  return Map<Y, X>(f);
}

template <typename X>
Map<X, X> visit(const std::function<void(X)>& f) {
  return Map<X, X>([f](const X& x) {
    f(x);
    return x;
  });
}

// Filter items
template <typename X>
class Filter {
public:
  Filter(const std::function<bool(X)> f) : _f(f) {}

  template <typename S>
  class Step : public TransducerStep<typename S::ResultType, X> {
  public:
    typedef typename S::ResultType R;

    Step(std::function<bool(X)> f, S s) : _f(f), _s(s) {}

    R step(R r, X x) {
      if (_f(x)) {
        return _s.step(r, x);
      } else {
        return r;
      }
    }

    R complete(R r) {
      return _s.complete(r);
    }
  private:
    std::function<bool(X)> _f;
    S _s;
  };

  template <typename S>
  Step<S> apply(const S& s) {
    return Step<S>(_f, s);
  }
private:
  std::function<bool(X)> _f;
};

template <typename F>
Filter<typename FunctionTraits<F>::input_type> filter(F f) {
  typedef FunctionTraits<F> T;
  return Filter<typename T::input_type>(f);
}

// Concatentate collections
template <typename Coll>
class Cat {
public:
  typedef typename Coll::value_type X;

  template <typename S>
  class Step : public TransducerStep<typename S::ResultType, Coll> {
  public:
    Step(S s) : _s(s) {}

    typedef typename S::ResultType R;

    R complete(R r) {return _s.complete(r);}
    R step(R r, Coll c) {
      R dst = r;
      for (auto x: c) {
        dst = _s.step(dst, x);
      }
      return dst;
    }
  private:
    S _s;
  };

  template <typename S>
  Step<S> apply(S s) {return Step<S>(s);}
};

template <typename T>
struct SpanWithCount {
  T first, last;
  int count = 0;
  SpanWithCount(T f, T l, int c) : first(f), last(l), count(c) {}
};

// Group similar data
template <typename T>
class FindSpans {
public:
  typedef std::function<bool(T, T)> Same;

  FindSpans(const Same& same) : _same(same) {}

  template <typename S>
  class Step : public TransducerStep<typename S::ResultType,
    SpanWithCount<T>> {
  public:
    typedef typename S::ResultType R;

    Step(S s, Same same) : _step(s), _same(same) {}

    SpanWithCount<T> current() const {
      return SpanWithCount<T>(_first, _last, _counter);
    }

    R complete(R r) {
      return _step.complete(_hasData?
        _step.step(r, current()) : r);
    }

    R step(R r, T x) {
      if (_hasData) {
        if (_same(_last, x)) {
          _last = x;
          _counter++;
          return r;
        } else {
          auto result = _step.step(r, current());
          reset(x);
          return result;
        }
      } else {
        _hasData = true;
        reset(x);
        return r;
      }
    }
  private:
    void reset(T x) {
      _counter = 1;
      _first = x;
      _last = x;
    }

    int _counter = 0;
    bool _hasData = false;
    T _first, _last;
    S _step;
    Same _same;
  };

  template <typename S>
  Step<S> apply(S s) {return Step<S>(s, _same);}
private:
  Same _same;
};

// Whenever the user-provided function returns true,
// start a new bundle
template <typename T>
class Bundle {
public:
  typedef std::function<bool(T)> StartNewBundle;

  Bundle(
      const StartNewBundle& startNewBundle)
    : _startNewBundle(startNewBundle) {}

  template <typename S>
  class Step : public TransducerStep<
    typename S::ResultType, Array<S>> {
  public:
    typedef typename S::ResultType R;

    Step(S s, const StartNewBundle& b) : _step(s), _b(b) {}

    R complete(R r) {
      return _step.complete(_current.empty()?
          r : _step.step(r, _current.get()));
    }

    R step(R r, T x) {
      if (_b(x)) {
        auto data = _current.get();
        _current = ArrayBuilder<T>();
        _current.add(x);
        return data.empty()? r : _step.step(r, data);
      } else if (!_current.empty()) {
        _current.add(x);
        return r;
      } else {
        return r;
      }
    }
  private:
    S _step;
    StartNewBundle _b;
    ArrayBuilder<T> _current;
  };

  template <typename S>
  Step<S> apply(S s) const {
    return Step<S>(s, _startNewBundle);
  }

private:
  StartNewBundle _startNewBundle;
};


/////////////// Transducer composition

// Composing two transducers results in a new transducer
template <typename A, typename B>
class ComposeTransducers2 {
public:
  ComposeTransducers2(const A& a, const B& b) : _a(a), _b(b) {}

  template <typename S>
  auto apply(const S& b)
    -> decltype(std::declval<A>().apply(
        std::declval<B>().apply(b))) {
    return _a.apply(_b.apply(b));
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
  auto apply(const S& s) -> decltype(
      std::declval<Inner>().apply(std::declval<S>())) {
    return _inner.apply(s);
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

////////////////////////// Reducing it
template <typename Step, typename ResultType, typename Iter>
ResultType reduce(Step s, ResultType init,
    Iter begin, Iter end) {
  ResultType result = init;
  for (auto i = begin; i != end; i++) {
    result = s.step(result, *i);
  }
  return s.complete(result);
}

template <typename Step, typename ResultType, typename Coll>
ResultType reduce(Step s, ResultType init, const Coll& c) {
  return reduce(s, init, c.begin(), c.end());
}

template <typename Transducer, typename DstColl, typename SrcColl>
void reduceIntoCollection(
    Transducer t, DstColl* dst, const SrcColl& src) {
  auto iter = std::inserter(*dst, dst->end());
  reduce(t.apply(iteratorStep(iter)), iter, src);
}

}



#endif /* SERVER_COMMON_TRANSDUCER_H_ */
