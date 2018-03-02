/*
 * RealPerfSurf.h
 *
 *  Created on: 2 Mar 2018
 *      Author: jonas
 *
 * This is glue code needed to perform a full polar computation
 * on a dataset.
 *
 */

#ifndef SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_
#define SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_

#include <server/common/Transducer.h>
#include <server/common/TimedValue.h>
#include <iostream>

namespace sail {

template <typename Iterator>
class TimedValuePairsStep {
public:
  // Obs: butEnd is a *valid iterator*. We can dereference it.
  // It is the iterator pointing just before the end of the range.
  TimedValuePairsStep(Iterator begin, Iterator butEnd)
    : _begin(begin), _butEnd(butEnd) {}


  template <typename Result, typename X>
  void apply(Result* r, X x) {
    if (_begin >= _butEnd || x.time < _begin->time) {
      return;
    }

    // Advance
    while (_begin < _butEnd && (_begin+1)->time <= x.time) {
      _begin++;
      addPending(r);
      _counter = 0;
    }
    if (_begin == _butEnd) {
      return;
    }

    if (_counter == 0) {
      r->add({*_begin, x});
    }
    _pending = x;

    _counter++;
  }

  template <typename Result>
  void flush(Result* r) {
    if (_begin < _butEnd) {
      _begin++;
      addPending(r);
    }
    r->flush();
  }
private:
  template <typename R>
  void addPending(R* r) {
    if (0 < _counter) {
      r->add({*_begin, _pending});
    }
  }
  typename Iterator::value_type _pending;
  int _counter = 0;
  Iterator _begin, _butEnd;
};

template <typename Iterator>
GenericTransducer<TimedValuePairsStep<Iterator>> trTimedValuePairs(
    Iterator b, Iterator e0) {
  auto butEnd = e0;
  butEnd--;
  return genericTransducer(TimedValuePairsStep<Iterator>(b, butEnd));
}


} /* namespace sail */

#endif /* SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_ */
