/*
 * TimedValueCutter.h
 *
 *  Created on: 25 Aug 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TIMEDVALUECUTTER_H_
#define SERVER_COMMON_TIMEDVALUECUTTER_H_

#include <server/common/Span.h>
#include <iterator>
#include <server/common/ArrayBuilder.h>
#include <server/common/TimedValue.h>

namespace sail {

template <typename TimedValueIterator,
  typename T=typename std::iterator_traits<TimedValueIterator>::value_type>
Array<Array<T> > cutTimedValues(
    TimedValueIterator from, TimedValueIterator to,
    const Array<Span<TimeStamp> > &spans) {
  Array<Array<T>> result(spans.size());
  auto at = from;
  for (int i = 0; i < spans.size(); i++) {
    auto span = spans[i];

    // Step forward until we get inside the current span.
    while (at != to && at->time < span.minv()) {
      at++;
    }
    if (at == to) {
      break;
    }

    ArrayBuilder<T> subResult;
    // Step within the current span
    while (at != to && at->time < span.maxv()) {
      subResult.add(*at);
      at++;
    }
    result[i] = subResult.get();
  }
  return result;
}

}



#endif /* SERVER_COMMON_TIMEDVALUECUTTER_H_ */
