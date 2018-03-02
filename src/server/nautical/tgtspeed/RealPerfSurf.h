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

namespace sail {

// This is a stateful transducer,
// but there is no state to flush.
template <typename Iterator, typename Step>
class TimedValuePairs : public Step,
  public Transducer<TimedValuePairs<Iterator, Step>> {
public:
  // Obs: butEnd is a *valid iterator*. We can dereference it.
  // It is the iterator pointing just before the end of the range.
  TimedValuePairs(Iterator begin, Iterator butEnd,
      const Step& s = Step()) : _begin(begin), _butEnd(butEnd), Step(s) {}

  typedef typename Step::input_type input_type;
  typedef typename Step::result_type result_type;

  // Use it as a step function
  result_type step(result_type y, input_type x) {
    if (_begin >= _butEnd || x.time < _begin->time) {
      return y;
    }


    // Advance
    while (_begin < _butEnd && (_begin+1)->time <= x.time) {
      _begin++;
    }
    if (_begin == _butEnd) {
      return y;
    }

    return Step::step(Step::step(y, {*_begin, x}), {*_butEnd, x});
  }

  // Use it as a transducer
  template <typename S>
  TimedValuePairs<Iterator, S> apply(S s) const {
    return TimedValuePairs<Iterator, S>(_begin, _butEnd, s);
  }
private:
  Iterator _begin, _butEnd;
};

template <typename Iterator>
TimedValuePairs<Iterator, UndefinedStep> trTimedValuePairs(
    Iterator b, Iterator e0) {
  auto butEnd = e0;
  butEnd--;
  return TimedValuePairs<Iterator, UndefinedStep>(b, butEnd);
}


} /* namespace sail */

#endif /* SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_ */
