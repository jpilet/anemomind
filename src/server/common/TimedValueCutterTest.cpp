/*
 * TimedValueCutterTest.cpp
 *
 *  Created on: 25 Aug 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/TimedValueCutter.h>

using namespace sail;

namespace {

  auto offset = TimeStamp::UTC(2016, 8, 25, 12, 21, 0);

  TimedValue<int> tv(double s, int i) {
    return TimedValue<int>(offset +
        Duration<double>::seconds(s), i);
  }

  Span<TimeStamp> span(double a, double b) {
    return Span<TimeStamp>(offset + Duration<double>::seconds(a),
        offset + Duration<double>::seconds(b));
  }

}

TEST(TimedValueCutter, CutEmpty) {
  Array<TimedValue<int> > valuesToCut;

  Array<Span<TimeStamp> > spans{span(3, 4)};
  auto result =
      cutTimedValues(valuesToCut.begin(), valuesToCut.end(), spans);
  EXPECT_EQ(result.size(), 1);
  EXPECT_TRUE(result[0].empty());
}

TEST(TimedValueCutter, CutOutside) {
  Array<TimedValue<int> > valuesToCut{tv(2.0, 3)};

  Array<Span<TimeStamp> > spans{span(3, 4)};
  auto result =
      cutTimedValues(valuesToCut.begin(), valuesToCut.end(), spans);
  EXPECT_EQ(result.size(), 1);
  EXPECT_TRUE(result[0].empty());
}

TEST(TimedValueCutter, CutOutside2) {
  Array<TimedValue<int> > valuesToCut{tv(5.0, 3)};

  Array<Span<TimeStamp> > spans{span(3, 4)};
  auto result =
      cutTimedValues(valuesToCut.begin(), valuesToCut.end(), spans);
  EXPECT_EQ(result.size(), 1);
  EXPECT_TRUE(result[0].empty());
}

TEST(TimedValueCutter, CutCovering) {
  Array<TimedValue<int> > valuesToCut{tv(3.5, 3)};

  Array<Span<TimeStamp> > spans{span(3, 4)};
  auto result =
      cutTimedValues(valuesToCut.begin(), valuesToCut.end(), spans);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0].size(), 1);
}



