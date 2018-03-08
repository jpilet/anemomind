/*
 * TimedTuples.cpp
 *
 *  Created on: 8 Mar 2018
 *      Author: jonas
 */

#include "TimedTuples.h"
#include <gtest/gtest.h>

using namespace sail;

TimeStamp t(double v) {
  return TimeStamp::UTC(2018, 3, 7, 11, 2, 0) + Duration<double>::seconds(v);
}

int counter = 0;

typedef TimedTuples::Indexed<int> Indexed;

Indexed indexed(int type) {
  return Indexed(type, counter++);
}

TEST(TimedTuplesTest, TestWithTwo) {
  std::vector<TimedValue<Indexed>> values{
    TimedValue<Indexed>(t(0), indexed(0)),
    TimedValue<Indexed>(t(1), indexed(0)),  // P1
    TimedValue<Indexed>(t(2), indexed(1)),  // P1
    TimedValue<Indexed>(t(3), indexed(1)),
    TimedValue<Indexed>(t(4), indexed(1)),  // P2
    TimedValue<Indexed>(t(5), indexed(0))   // P2
  };



  transduce(
      values,
      timedTuples<int, 2>(),
      IntoArray<std::array<TimedValue<int>, 2>>());

}
