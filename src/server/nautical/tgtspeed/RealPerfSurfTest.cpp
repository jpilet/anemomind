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

TEST(RealPerfSurfTest, TimedValuePairs2) {
  std::vector<TimedValue<int>> A{
    tv(0.0, 1),
  };

  std::vector<TimedValue<int>> B{
    tv(1.0, 119),
  };

  auto dst = transduce(
      B,
      trTimedValuePairs(A.begin(), A.end()),
      IntoArray<std::pair<TimedValue<int>, TimedValue<int>>>());

  EXPECT_TRUE(dst.empty());
}

TEST(RealPerfSurfTest, TimedValuePairs3) {
  std::vector<TimedValue<int>> A{
    tv(0.0, 1),
    tv(2.0, 3)
  };

  std::vector<TimedValue<int>> B{
    tv(1.0, 119),
    tv(1.1, 98),
    tv(1.3, 445),
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
  EXPECT_EQ(b.second.value, 445);
}

TEST(RealPerfSurfTest, TimedValuePairs4) {
  std::vector<TimedValue<int>> A{
    tv(0.0, 1),
    tv(2.0, 3),
    tv(3.0, 9),
  };

  std::vector<TimedValue<int>> B{
    tv(1.0, 119),
    tv(2.5, 445),
  };

  auto dst = transduce(
      B,
      trTimedValuePairs(A.begin(), A.end()),
      IntoArray<std::pair<TimedValue<int>, TimedValue<int>>>());

  EXPECT_EQ(dst.size(), 4);
  auto a = dst[0];
  auto b = dst[1];
  auto c = dst[2];
  auto d = dst[3];
  EXPECT_EQ(a.first.value, 1);
  EXPECT_EQ(b.first.value, 3);
  EXPECT_EQ(c.first.value, 3);
  EXPECT_EQ(d.first.value, 9);
}

TEST(RealPerfSurfTest, CompositeProcessing) {
  std::vector<TimedValue<int>> A{
    tv(0.0, 1),
    tv(10.0, 3),
  };

  std::vector<TimedValue<int>> B{
    tv(1.0, 119),
  };

  auto dst = transduce(
       B,
       trTimedValuePairs(A.begin(), A.end())
       |
       trFilter(IsTightTimePair(2.0_seconds))
       |
       trMap(CollapseTimePair()),
       IntoArray<TimedValue<std::pair<int, int>>>());

  EXPECT_EQ(dst.size(), 1);
  auto x = dst[0];
  EXPECT_NEAR((x.time - tv(0.5, 0).time).seconds(), 0.0, 0.02);
  EXPECT_EQ(x.value.first, 1);
  EXPECT_EQ(x.value.second, 119);

}


