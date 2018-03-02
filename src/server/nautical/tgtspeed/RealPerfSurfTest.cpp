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

  auto dst = transduce(
      B,
      trTimedValuePairs(A.begin(), A.end()),
      IntoArray<std::pair<TimedValue<int>, TimedValue<int>>>());

  EXPECT_EQ(dst.size(), 2);
  auto a = dst[0];
  auto b = dst[1];

  EXPECT_EQ(a.first.value, 1);
  EXPECT_EQ(a.second.value, 119);

  EXPECT_EQ(b.first.value, 3);
  EXPECT_EQ(b.second.value, 119);

}



