/*
 * SmoothGpsFilterTest.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/Env.h>
#include <server/common/logging.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/WGS84.h>
#include <server/common/DOMUtils.h>
#include <server/common/ArrayIO.h>


using namespace sail;



namespace {

NavDataset getPsarosTestData() {
  auto p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("psaros33_Banque_Sturdza")
    .pushDirectory("2014")
    .pushDirectory("20140821").get();
  LogLoader loader;
  loader.load(p.toString());
  auto ds = loader.makeNavDataset().fitBounds();
  auto startAt = TimeStamp::UTC(2014, 8, 21, 16, 46, 30);
  return ds.sliceFrom(startAt);
}

NavDataset applyOutliersAsBefore(NavDataset navs) {
  LOG(FATAL) << "TODO: Adapt this code for the dispatcher";


  // TODO: Adapt this code for the dispatcher
  //       Our refactored GPS processing code should successfully be able to filter this sort of data,
  //       or at least identify the outliers inside it.

        /*  int from = int(floor(getNavSize(navs)*0.05));
          int to = int(floor(getNavSize(navs)*0.15));
          auto dst = makeArray(navs).dup();
          Angle<double> offset = Angle<double>::degrees(1);
          int n = getNavSize(navs);
          for (int i = 0; i < n; i++) {
            if (i % 10 == 0) {
              auto &x = dst[i];
              auto g = x.geographicPosition();
              auto g2 = GeographicPosition<double>(g.lon(), g.lat() + offset);
              x.setGeographicPosition(g2);
            }
          }
          return fromNavs(dst);*/
  return navs;
}

TimedSampleCollection<GeographicPosition<double> >::TimedVector applyOutliers(
    const TimedSampleRange<GeographicPosition<double> > &src) {
  TimedSampleCollection<GeographicPosition<double> >::TimedVector corruptPositions;
  int n = src.size();
  CHECK(60 < n);
  for (int i = 0; i < n; i++) {
    const auto &x = src[i];
    if (i % 10 == 0) {
      auto lon = x.value.lon() + Angle<double>::degrees(13.0234*sin(cos(74.9324*i) + 234.433*sin(324.0*i)));
      corruptPositions.push_back(TimedValue<GeographicPosition<double> >(x.time,
          GeographicPosition<double>(lon, x.value.lat(), x.value.alt())));
    } else {
      corruptPositions.push_back(x);
    }
  }
  return corruptPositions;
}

TimeStamp timeStamp(double x) {
  auto offset = TimeStamp::UTC(2016, 6, 8, 13, 52, 0);
  return offset + Duration<double>::seconds(x);
}


TimedValue<Curve2dFilter::Vec2<Length<double>>> timedPos(double t, double x, double y) {
  return TimedValue<Curve2dFilter::Vec2<Length<double>>>(
      timeStamp(t), {
          x*1.0_m, y*1.0_m
  });
}

}

TEST(SmoothGpsFilterTest, TestIt) {
  auto original = getPsarosTestData();
  EXPECT_FALSE(original.isDefaultConstructed());
  auto originalPositions = original.samples<GPS_POS>();
  auto corruptedPositions = applyOutliers(originalPositions);

  EXPECT_EQ(originalPositions.size(), corruptedPositions.size());
  int n = originalPositions.size();


  auto srcName = "Test corrupted";

  NavDataset corrupted = original.replaceChannel<GeographicPosition<double>>(
      GPS_POS, srcName, corruptedPositions);

  // This test case is broken. An assertion is fired in debug mode.
  // It is fixed by https://github.com/jpilet/anemomind/pull/698
  DOM::Node out;
  auto filtered0 = filterGpsData(corrupted, &out);

  EXPECT_FALSE(filtered0.positions.empty());

  auto filteredPositions = filtered0.positions;


  int filteredCounter = 0;
  int corrCounter = 0;
  int marg = 3;
  for (int i = 0; i < n; i++) {
    if (i % 10 == 0) {
      auto good = originalPositions[i];
      auto bad = corruptedPositions[i];
      auto t = good.time;

      // Step to the next filtered sample that is close to our good/bad ones.
      while (filteredCounter < filteredPositions.size() &&
          filteredPositions[filteredCounter].time < t) {
        filteredCounter++;
      }

      for (int j = std::max(0, filteredCounter - marg);
          j <= std::min<int>(filteredPositions.size()-1, filteredCounter + marg); j++) {
        auto x = filteredPositions[j];

        auto distToGood = distance(x.value, good.value);
        auto distToBad = distance(x.value, bad.value);
        corrCounter++;
        EXPECT_LT(distToGood, distToBad); // TODO: Actually quite weak test, but better than nothing.

        EXPECT_LT(distToGood.meters(), 30);
      }

      if (filteredCounter == filteredPositions.size()) {
        break;
      }
    }
  }
  EXPECT_LT(60, corrCounter);
}

namespace {
  auto offset = TimeStamp::UTC(2016, 8, 12, 10, 17, 0);
  auto s = Duration<double>::seconds(1.0);

  TimeStamp lt(double t) {
    return offset + t*s;
  }

  TimedValue<int> tvi(double t, int x) {
    return TimedValue<int>{lt(t), x};
  }


}

namespace sail {
  bool operator==(const TimedValue<int> &a, const TimedValue<int> &b) {
    return a.time == b.time && a.value == b.value;
  }
}

TEST(SmoothGpsFilter, SplittingTimeStamps) {
  Array<TimeStamp> T{lt(0.0), lt(0.3), lt(1.1)};
  auto splits = listSplittingTimeStamps(T, 0.4*s);
  EXPECT_EQ(splits.size(), 1);
  EXPECT_NEAR((splits[0] - offset)/s, 0.7, 1.0e-6);
}

TEST(SmoothGpsFilter, ApplySplits) {
  auto emptyArray = Array<TimedValue<int> >();
  {
      Array<TimeStamp> times;
      Array<TimedValue<int> > values{tvi(0.1, 3), tvi(0.2, 4)};
      EXPECT_TRUE((Array<Array<TimedValue<int>>>{values}) ==
          applySplits(values, times));
  }{
    Array<TimeStamp> times{lt(-0.3)};
    Array<TimedValue<int> > values{tvi(0.1, 3), tvi(0.2, 4)};
    EXPECT_TRUE((Array<Array<TimedValue<int>>>{emptyArray, values}) ==
        applySplits(values, times));
  }{
    Array<TimeStamp> times{lt(0.9)};
    Array<TimedValue<int> > values{tvi(0.1, 3), tvi(0.2, 4)};
    EXPECT_TRUE((Array<Array<TimedValue<int>>>{values, emptyArray}) ==
        applySplits(values, times));
  }{
    Array<TimeStamp> times{lt(0.15)};
    Array<TimedValue<int> > values{tvi(0.1, 3), tvi(0.2, 4)};
    EXPECT_TRUE((Array<Array<TimedValue<int>>>{
      Array<TimedValue<int>>{tvi(0.1, 3)},
      Array<TimedValue<int>>{tvi(0.2, 4)},
    }) ==
    applySplits(values, times));
  }
}
