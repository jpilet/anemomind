/*
 * TimedValue.h
 *
 *  Created on: Apr 14, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TIMEDVALUE_H_
#define SERVER_COMMON_TIMEDVALUE_H_

#include <server/common/TimeStamp.h>

namespace sail {

template <typename T>
struct TimedValue {
  typedef T type;

  TimedValue() {}
  TimedValue(TimeStamp time, T value) : time(time), value(value) { }

  TimeStamp time;
  T value;

  bool operator < (const TimedValue<T>& other) const {
    return time < other.time;
  }
};

template <typename T>
bool operator<(const TimedValue<T> &a, TimeStamp b) {
  return a.time < b;
}

template <typename T>
bool operator<(const TimeStamp &a, const TimedValue<T> &b) {
  return a < b.time;
}

template <typename TimedValueType>
struct WrappedTimedValueType {
public:
  typedef decltype((std::declval<TimedValueType>()).value) type;
};

}



#endif /* SERVER_COMMON_TIMEDVALUE_H_ */
