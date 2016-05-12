/*
 * TimedValuePairsTest.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/TimedValuePairs.h>

using namespace sail;

namespace {
  auto offset = TimeStamp::UTC(2016, 5, 12, 10, 13, 0);

  auto seconds = Duration<double>::seconds(1.0);

  template <typename T>
  TimedValue<T> tv_(double localSeconds, T value) {
    return TimedValue<T>(offset + localSeconds*seconds, value);
  }

  TimedValue<int> tv(double t, int v) {
    return tv_<int>(t, v);
  }

  TimedValue<std::string> tv(double t, std::string v) {
    return tv_<std::string>(t, v);
  }

  typedef std::pair<TimedValue<int>, TimedValue<std::string> > Pair;


  template <typename T>
  bool operator==(const TimedValue<T> &a, const TimedValue<T> &b) {
    return a.time == b.time && a.value == b.value;
  }

  bool operator==(const Pair &a, const Pair &b) {
    return a.first == b.first && a.second == b.second;
  }
}


TEST(TVPTest, Empty) {
  Array<TimedValue<int> > A;
  Array<TimedValue<std::string> > B;
  auto C = TimedValuePairs::make(A.begin(), A.end(), B.begin(), B.end());
  EXPECT_TRUE(C.empty());
}

TEST(TVPTest, OneValueInA) {
  Array<TimedValue<int> > A{tv(0.0, 119)};
  Array<TimedValue<std::string> > B;
  auto C = TimedValuePairs::make(A.begin(), A.end(), B.begin(), B.end());
  EXPECT_TRUE(C.empty());
}

TEST(TVPTest, OneValueInB) {
  Array<TimedValue<int> > A;
  Array<TimedValue<std::string> > B{tv(0.0, "rulle")};
  auto C = TimedValuePairs::make(A.begin(), A.end(), B.begin(), B.end());
  EXPECT_TRUE(C.empty());
}

TEST(TVPTest, OnePair) {
  Array<TimedValue<int> > A{tv(0.0, 119)};
  Array<TimedValue<std::string> > B{tv(0.0, "rulle")};
  auto C = TimedValuePairs::make(A.begin(), A.end(), B.begin(), B.end());
  EXPECT_EQ(C.size(), 1);
  EXPECT_TRUE(Pair(C[0]) == (Pair{tv(0.0, 119), tv(0.0, "rulle")}));
}

TEST(TVPTest, TwoPairs) {
  Array<TimedValue<int> > A{tv(0.0, 119), tv(1.0, 120)};
  Array<TimedValue<std::string> > B{tv(0.1, "rulle")};
  auto C = TimedValuePairs::make(A.begin(), A.end(), B.begin(), B.end());
  EXPECT_EQ(C.size(), 2);
  EXPECT_TRUE(C == (Array<Pair>{
    Pair{tv(0.0, 119), tv(0.1, "rulle")},
    Pair{tv(1.0, 120), tv(0.1, "rulle")}
  }));
}

TEST(TVPTest, TwoSeparatePairs) {
  Array<TimedValue<int> > A{tv(0.0, 119), tv(1.0, 120)};
  Array<TimedValue<std::string> > B{tv(0.1, "rulle"), tv(0.2, "bulle")};
  auto C = TimedValuePairs::make(A.begin(), A.end(), B.begin(), B.end());
  EXPECT_EQ(C.size(), 2);
  EXPECT_TRUE(C == (Array<Pair>{
    Pair{tv(0.0, 119), tv(0.1, "rulle")},
    Pair{tv(1.0, 120), tv(0.2, "bulle")}
  }));
}

TEST(TVPTest, TwoSeparatePairsAndOneUnusedSample) {
  Array<TimedValue<int> > A{tv(0.0, 119), tv(1.0, 120)};
  Array<TimedValue<std::string> > B{tv(0.1, "rulle"), tv(0.15, "unused"), tv(0.2, "bulle")};
  auto C = TimedValuePairs::make(A.begin(), A.end(), B.begin(), B.end());
  EXPECT_EQ(C.size(), 2);
  EXPECT_TRUE(C == (Array<Pair>{
    Pair{tv(0.0, 119), tv(0.1, "rulle")},
    Pair{tv(1.0, 120), tv(0.2, "bulle")}
  }));
}
