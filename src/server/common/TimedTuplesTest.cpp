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

typedef TimedTuples::Indexed<int> Indexed;

Indexed indexed(int type, int v) {
  return Indexed(type, v);
}

TEST(TimedTuplesTest, TestWithTwo) {
  std::vector<TimedValue<Indexed>> values{
    TimedValue<Indexed>(t(0), indexed(0, 0)),
    TimedValue<Indexed>(t(1), indexed(0, 1)),  // P1
    TimedValue<Indexed>(t(2), indexed(1, 2)),  // P1
    TimedValue<Indexed>(t(3), indexed(1, 3)),
    TimedValue<Indexed>(t(4), indexed(1, 4)),  // P2
    TimedValue<Indexed>(t(5), indexed(0, 5))   // P2
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
    TimedValue<Indexed>(t(0), indexed(0, 0)),
    TimedValue<Indexed>(t(0.99), indexed(0, 1)),
    TimedValue<Indexed>(t(2), indexed(1, 2)), // 1
    TimedValue<Indexed>(t(3), indexed(2, 3)), // 1
    TimedValue<Indexed>(t(4), indexed(1, 4)), // 1
    TimedValue<Indexed>(t(5), indexed(0, 5)),
    TimedValue<Indexed>(t(6), indexed(0, 6)), // 2
    TimedValue<Indexed>(t(7), indexed(2, 7)), // 2
    TimedValue<Indexed>(t(8), indexed(1, 8)), // 2
    TimedValue<Indexed>(t(9.1), indexed(0, 9))
  };
  auto result = transduce(
      values,
      trTimedTuples<int, 3>(),
      IntoArray<std::array<TimedValue<int>, 3>>());
  EXPECT_EQ(result.size(), 2);
}
