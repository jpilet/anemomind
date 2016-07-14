/*
 * ResamplerTest.cpp
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#include <iostream>
#include <server/common/ArrayIO.h>
#include <server/math/Resampler.h>
#include <gtest/gtest.h>
#include <server/common/Functional.h>

using namespace sail;
using namespace sail::Resampler;

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
    return sail::map(x, &toTime);
  }

  Arrayi fromTimeArray(const Array<TimeStamp> &x) {
    return sail::map(x, &fromTime);
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
