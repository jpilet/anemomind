/*
 * TimedTuples.cpp
 *
 *  Created on: 8 Mar 2018
 *      Author: jonas
 */

#include "TimedTuples.h"
#include <gtest/gtest.h>
#include <tuple>

using namespace sail;

TimeStamp t(double v) {
  return TimeStamp::UTC(2018, 3, 7, 11, 2, 0) + Duration<double>::seconds(v);
}

typedef IndexedValue<int> Indexed;

Indexed indexed(int type, int v) {
  return Indexed(type, v);
}

TimedValue<Indexed> testValue(double index, int type) {
  return TimedValue<Indexed>(t(index),
      indexed(type, int(round(index))));
}

TEST(TimedTuplesTest, TestWithTwo) {
  std::vector<TimedValue<Indexed>> values{
    testValue(0, 0),
    testValue(1, 0),  // P1
    testValue(2, 1),  // P1
    testValue(3, 1),
    testValue(4, 1),  // P2
    testValue(5, 0)   // P2
  };

  auto result = transduce(
      values,
      trTimedTuples<int, 2>(),
      IntoArray<std::array<TimedValue<int>, 2>>());

  EXPECT_EQ(result.size(), 2);
  {
    auto x = result[0];
    EXPECT_EQ(x[0].time, t(1));
    EXPECT_EQ(x[0].value, 1);

    EXPECT_EQ(x[1].time, t(2));
    EXPECT_EQ(x[1].value, 2);
  }{
    auto x = result[1];
    EXPECT_EQ(x[0].time, t(5));
    EXPECT_EQ(x[0].value, 5);

    EXPECT_EQ(x[1].time, t(4));
    EXPECT_EQ(x[1].value, 4);
  }
}

void testBinaryTuplesForHistoryLength(
    std::vector<TimedValue<Indexed>> values,
    int len) {
  TimedTuples::Settings settings;
  settings.halfHistoryLength = len;

  auto result = transduce(
      values,
      trTimedTuples<int, 2>(settings),
      IntoArray<std::array<TimedValue<int>, 2>>());

  EXPECT_EQ(result.size(), 10);

  {
    auto x = result[7];
    EXPECT_EQ(x[0].time, t(23));
    EXPECT_EQ(x[0].value, 23);

    EXPECT_EQ(x[1].time, t(22));
    EXPECT_EQ(x[1].value, 22);
  }{
    auto x = result[8];
    EXPECT_EQ(x[0].time, t(25));
    EXPECT_EQ(x[0].value, 25);

    EXPECT_EQ(x[1].time, t(26));
    EXPECT_EQ(x[1].value, 26);
  }
}

TEST(TimedTuplesTest, TestWithIntermediateFlush) {
  std::vector<TimedValue<Indexed>> values;

  int counter = 0;
  std::array<int, 6> classes{0, 0, 1, 1, 1, 0};
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 6; j++) {
      auto c = classes[j];
      values.push_back(TimedValue<Indexed>(
          t(counter), indexed(c, counter)));
      std::cout << "  t=" << counter << "  c=" << c << std::endl;
      counter++;
    }
  }
  EXPECT_EQ(values.size(), 30);

  for (int i = 2; i < 40; i++) {
    testBinaryTuplesForHistoryLength(values, i);
  }
}

TEST(TimedTuplesTest, TestWithThree) {
  std::vector<TimedValue<Indexed>> values{
    testValue(0, 0),
    testValue(0.99, 0),  // 1
    testValue(2, 1), // 1
    testValue(3, 2), // 1
    testValue(4, 1),
    testValue(5.1, 0),
    testValue(6, 0), // 2
    testValue(7, 2), // 2
    testValue(8, 1), // 2
    testValue(9.1, 0)
  };
  auto result = transduce(
      values,
      trTimedTuples<int, 3>(),
      IntoArray<std::array<TimedValue<int>, 3>>());
  EXPECT_EQ(result.size(), 2);

  {
    auto x = result[0];
    EXPECT_EQ(x[0].value, 1);
    EXPECT_EQ(x[1].value, 2);
    EXPECT_EQ(x[2].value, 3);
  }
  {
    auto x = result[1];
    EXPECT_EQ(x[0].value, 6);
    EXPECT_EQ(x[1].value, 8);
    EXPECT_EQ(x[2].value, 7);
  }
}

TEST(TimedTuplesTest, EmptyTest) {
  std::vector<TimedValue<Indexed>> values;
  auto result = transduce(
      values,
      trTimedTuples<int, 3>(),
      IntoArray<std::array<TimedValue<int>, 3>>());
  EXPECT_EQ(result.size(), 0);
}


TEST(TimedTuplesTest, TestWithTwoAndFiltering) {
  std::vector<TimedValue<Indexed>> values{
    testValue(0, 0),
    testValue(1, 0),  // P1
    testValue(1.9, 1),  // P1
    testValue(3, 1),
    testValue(4, 1),  // P2: Rejected, too long.
    testValue(5.2, 0)   // P2
  };

  auto result = transduce(
      values,
      trTimedTuples<int, 2>()
      |
      trFilter(IsShortTimedTuple(1.0_s)),
      IntoArray<std::array<TimedValue<int>, 2>>());

  EXPECT_EQ(result.size(), 1);
  {
    auto x = result[0];
    EXPECT_EQ(x[0].value, 1);

    EXPECT_EQ(x[1].value, 2);
  }
}
