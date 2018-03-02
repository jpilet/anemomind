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

namespace sail {

/*template <typename Iterator, typename Step>
class TimedValueMerge : public Step,
  public Transducer<TimedValueMerge<Iterator, Step>> {
public:
  TimedValueMerge(Iterator begin, Iterator end,
      const Step& s = Step()) : _begin(begin), _end(end), Step(s) {}

  typedef typename Step::input_type input_type;
  typedef typename Step::result_type result_type;

  // Use it as a step function
  result_type step(result_type y, input_type x) {
    if (_begin == _end) {
      return y;
    } else {

    }
    return _f(x)? Step::step(y, x) : y;
  }

  // Use it as a transducer
  template <typename S>
  TimedValueMerge<Iterator, S> apply(S s) const {
    return TimedValueMerge<Iterator, S>(_begin, _end, s);
  }
private:
  Iterator _begin, _end;
};

template <typename Iterator>
TimedValueMerge<Iterator, UndefinedStep> trTimedValueMerge(
    Iterator b, Iterator e) {
  return TimedValueMerge<Iterator, UndefinedStep>(b, e);
}*/


} /* namespace sail */

#endif /* SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_ */
