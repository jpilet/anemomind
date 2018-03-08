/*
 * TimedTuples.cpp
 *
 *  Created on: 8 Mar 2018
 *      Author: jonas
 */

#include "TimedTuples.h"
#include <gtest/gtest.h>
#include <tuple>
#include <server/common/VariantIterator.h>

using namespace sail;

TimeStamp t(double v) {
  return TimeStamp::UTC(2018, 3, 7, 11, 2, 0) + Duration<double>::seconds(v);
}

typedef IndexedValue<int> Indexed;

Indexed indexed(int type, int v) {
  return Indexed(type, v);
}

TimedValue<IndexedValue<int>> testValue(double index, int type) {
  return TimedValue<IndexedValue<int>>(
      t(index), {type, int(round(index))});
}

TEST(TimedTuplesTest, TestWithTwo) {
  std::vector<TimedValue<IndexedValue<int>>> values{
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
    std::vector<TimedValue<IndexedValue<int>>> values,
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
  std::vector<TimedValue<IndexedValue<int>>> values;

  int counter = 0;
  std::array<int, 6> classes{0, 0, 1, 1, 1, 0};
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 6; j++) {
      auto c = classes[j];
      values.push_back(testValue(counter, c));
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
  std::vector<TimedValue<IndexedValue<int>>> values{
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
  std::vector<TimedValue<IndexedValue<int>>> values;
  auto result = transduce(
      values,
      trTimedTuples<int, 3>(),
      IntoArray<std::array<TimedValue<int>, 3>>());
  EXPECT_EQ(result.size(), 0);
}


TEST(TimedTuplesTest, TestWithTwoAndFiltering) {
  std::vector<TimedValue<IndexedValue<int>>> values{
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

/*TEST(TimedTuplesTest, RealisticTest) {
  std::vector<TimedValue<Angle<double>>> angles{
    {t(0), 4.5_deg},
    {t(1), 4.7_deg},
    {t(3), 9.0_deg},
    {t(5), 8.8_deg},
    {t(7), 9.8_deg}
  };
  typedef VariantIteratorWrapper<
        0, Angle<double>, Velocity<double>> AwaWrap;
  auto awaWrap = AwaWrap();

  std::vector<TimedValue<Velocity<double>>> velocities{
    {t(0.1), 4.5_kn},
    {t(3.2), 4.8_kn},
    {t(8.2), 4.3_kn}
  };
  auto awsWrap = awaWrap.next();

  auto result = transduce(
      angles,
      trMap(awaWrap)
      |
      trMerge(
          awsWrap.wrap(velocities.begin()),
          awsWrap.wrap(velocities.end())),
      IntoArray<IndexedValue<TimedValue<AwaWrap::Variant>>>());

  EXPECT_EQ(result.size(), 5);
}*/
