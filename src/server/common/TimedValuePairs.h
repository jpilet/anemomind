/*
 * TimedValuePairs.h
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 *
 * Build pairs of timed values, e.g. angles and velocities to compose motions
 */
#include <iterator>
#include <server/common/TimedValue.h>
#include <server/common/logging.h>
#include <server/common/ArrayBuilder.h>
#include <algorithm>

#ifndef SERVER_COMMON_TIMEDVALUEPAIRS_H_
#define SERVER_COMMON_TIMEDVALUEPAIRS_H_

namespace sail {
namespace TimedValuePairs {

// Returns 0 for first array, 1 for second array, and -1 if undefined.
template <typename TVIterA, typename TVIterB>
int selectRangeToReadNextValueFrom(TVIterA aBegin, TVIterA aEnd, TVIterB bBegin, TVIterB bEnd) {
  if (aBegin == aEnd) {
    if (bBegin == bEnd) {
      return -1;
    }
    return 1;
  }
  if (bBegin == bEnd) {
    return 0;
  }
  auto tvA = (*aBegin);
  auto tvB = (*bBegin);
  if (!tvA.time.defined() || !tvB.time.defined()) {
    LOG(FATAL) << "Undefined times are not permitted";
    return -1;
  }
  return tvA.time < tvB.time? 0 : 1;
}

// The template parameters are TimedValue-iterator types
template <typename TVIterA, typename TVIterB>

Array<std::pair<typename std::iterator_traits<TVIterA>::value_type,
                typename std::iterator_traits<TVIterB>::value_type> >
  makeTimedValuePairs(TVIterA a0, TVIterA a1, TVIterB b0, TVIterB b1) {

  assert(std::is_sorted(a0, a1));
  assert(std::is_sorted(b0, b1));

  typedef typename std::iterator_traits<TVIterA>::value_type::type A;
  typedef typename std::iterator_traits<TVIterB>::value_type::type B;
  typedef std::pair<TimedValue<A>, TimedValue<B> > Pair;

  int last = -1;
  TimedValue<A> lastA;
  TimedValue<B> lastB;

  ArrayBuilder<Pair> pairs;

  while (true) {
    auto next = selectRangeToReadNextValueFrom(a0, a1, b0, b1);
    if (next == -1) {
      break;
    }
    if (next == 1) {
      auto nextB = *b0;
      b0++;
      if (last == 0) {
        pairs.add(Pair(lastA, nextB));
      }
      lastB = nextB;
    } else if (next == 0) {
      auto nextA = *a0;
      a0++;
      if (last == 1) {
        pairs.add(Pair(nextA, lastB));
      }
      lastA = nextA;
    }
    last = next;
  }
  return pairs.get();
}


}
}


#endif /* SERVER_COMMON_TIMEDVALUEPAIRS_H_ */
