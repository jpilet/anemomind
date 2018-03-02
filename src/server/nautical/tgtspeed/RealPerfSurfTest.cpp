/*
 * RealPerfSurfTest.cpp
 *
 *  Created on: 2 Mar 2018
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/tgtspeed/RealPerfSurf.h>

using namespace sail;

TimedValue<int> tv(double t, int value) {
  return TimedValue<int>(
      TimeStamp::UTC(2018, 3, 2, 15, 11, 0) + t*1.0_s, value);
}

TEST(RealPerfSurfTest, TimedValuePairs) {
  std::vector<TimedValue<int>> A{
    tv(0.0, 1),
    tv(2.0, 3)
  };

  std::vector<TimedValue<int>> B{
    tv(1.0, 119),
    tv(4.0, 120)
  };

  std::vector<std::pair<TimedValue<int>, TimedValue<int>>> dst;
  transduceIntoColl(trTimedValuePairs(A.begin(), A.end()), &dst, B);
}



