/*
 * ResamplerTest.cpp
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#include <iostream>
#include <server/common/ArrayIO.h>
#include <server/math/SampleUtils.h>
#include <gtest/gtest.h>
#include <server/transducers/Transducer.h>

using namespace sail;
using namespace sail::SampleUtils;

namespace {
  auto offset = TimeStamp::UTC(2016, 5, 20, 9, 59, 0);

  Duration<double> unit = Duration<double>::days(1.0);

  TimeStamp toTime(int i) {
    return offset + double(i)*unit;
  }

  int fromTime(TimeStamp t) {
    return int(round((t - offset).days()));
  }

  Array<TimeStamp> toTimeArray(const Arrayi &x) {
    return transduce(x, trMap(&toTime), IntoArray<TimeStamp>());
  }

  Arrayi fromTimeArray(const Array<TimeStamp> &x) {
    return transduce(x, trMap(&fromTime), IntoArray<int>());
  }
}

TEST(ResamplerTest, TestEps) {
  Arrayi samples{30, 40};
  auto period = 1.0*unit;
  auto eps = listEndpoints(toTimeArray(samples), period);
  EXPECT_EQ(eps.size(), 4);
  int times[4] = {29, 31, 39, 41};
  for (int i = 0; i < 4; i++) {
    EXPECT_EQ(toTime(times[i]), eps[i].pos);
    EXPECT_EQ(eps[i].rising, i % 2 == 0);
  }
  auto newSamples = fromTimeArray(
      makeNewSamplesFromEndpoints(eps, period));
  EXPECT_EQ(newSamples, (Arrayi{30, 40}));
}


TEST(ResamplerTest, TestEpsDense) {
  Arrayi samples{30, 31, 34, 35, 37, 40};
  auto period = 2.0*unit;
  auto eps = listEndpoints(toTimeArray(samples), period);
  EXPECT_EQ(eps.size(), 12);
  int times[12] = {28, 29, 32, 32, 33, 33, 35, 36, 37, 38, 39, 42};
  for (int i = 0; i < 12; i++) {
    EXPECT_EQ(toTime(times[i]), eps[i].pos);
  }
  auto newSamples = fromTimeArray(makeNewSamplesFromEndpoints(
      eps, period));


  auto expected = Arrayi{30, 32, 34, 36, 38, 40};
  EXPECT_EQ(newSamples, expected);
  EXPECT_EQ(
      fromTimeArray(resample(toTimeArray(samples), period)),
      expected);
}

TimedValue<bool> tv(int i, bool v) {
  return TimedValue<bool>(toTime(i), v);
}

TEST(SampleUtilsTest, GoodSpans) {
  Array<TimedValue<bool>> samples{
    tv(0, true),
    tv(1, true),
    tv(2, true),
    tv(3, true),
    tv(4, false),
    tv(5, true),
    tv(6, true),
    tv(7, true),
    tv(8, true)
  };
  auto spans = makeGoodSpans(samples, 1.5*unit, 1.5*unit);
  EXPECT_EQ(spans.size(), 2);
  auto a = spans[0];
  auto b = spans[1];

  EXPECT_EQ(fromTime(a.minv()), 0);
  EXPECT_EQ(fromTime(a.maxv()), 2);
  EXPECT_EQ(fromTime(b.minv()), 6);
  EXPECT_EQ(fromTime(b.maxv()), 8);
}
