/*
 * TimedValueUtilsTest.cpp
 *
 *  Created on: 10 Nov 2016
 *      Author: jonas
 */

#include <server/common/TimedValueUtils.h>
#include <gtest/gtest.h>

using namespace sail;

namespace {
  auto offset = TimeStamp::UTC(2016, 11, 10, 17, 50, 0);

  TimeStamp t(double s) {
    return offset + s*1.0_s;
  }
}

TEST(TimedValueUtils, TestIt) {
  auto bds = listAllBounds({t(0), t(1), t(2), t(3), t(9)}, 2.0_s);
  EXPECT_EQ((Array<int>{0, 4, 5}), bds);
  EXPECT_EQ((Array<int>{0}), (listAllBounds({}, 2.0_s)));

  {
    auto spans = listTimeSpans({t(0), t(1), t(4), t(5)}, 2.0_s, true);
    EXPECT_EQ(spans, (Array<Span<TimeStamp>>{
      Span<TimeStamp>(t(0), t(1)),
      Span<TimeStamp>(t(4), t(5))
    }));
  }{
    auto spans = listTimeSpans({t(0), t(1), t(4)}, 2.0_s, false);
    EXPECT_EQ(spans, (Array<Span<TimeStamp>>{
      Span<TimeStamp>(t(0), t(1))
    }));
  }
}

TEST(TimedValueUtils, AssignSpan) {
  auto spans = Array<Span<TimeStamp>>{
    Span<TimeStamp>{offset, offset + 2.0_s},
    Span<TimeStamp>{offset + 2.0_s, offset + 4.0_s}
  };

  auto times = Array<TimeStamp>{
    offset - 1.0_s,
    offset + 1.0_s,
    offset + 3.0_s,
    offset + 5.0_s
  };

  auto inds = getTimeSpanPerTimeStamp(spans, times);
  EXPECT_EQ(inds, (Array<int>{-1, 0, 1, -1}));
}

TEST(TimedValueUtils, FindNearest) {
  auto a = Array<TimeStamp>{
    offset - 1.0_s,
    offset + 1.0_s,
    offset + 3.0_s,
    offset + 9.0_s
  };

  auto b = Array<TimeStamp>{
    offset + 1.5_s,
    offset + 4.0_s
  };

  auto inds = findNearestTimePerTime(a, b);
  EXPECT_EQ(inds, (Array<int>{0, 0, 1, 1}));
}
