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


CeresTrajectoryFilter::Types<2>::TimedPosition timedPos(double t, double x, double y) {
  return CeresTrajectoryFilter::Types<2>::TimedPosition(timeStamp(t),
      Vectorize<Length<double>, 2>{Length<double>::meters(x), Length<double>::meters(y)});
}

}

TEST(SmoothGpsFilterTest, TestComputedMotions) {
  auto deg = Angle<double>::degrees(1.0);

  Array<CeresTrajectoryFilter::Types<2>::TimedPosition> raw{
      timedPos(0, 3, 4),
      timedPos(1, 3, -1)
    };

  Array<CeresTrajectoryFilter::Types<2>::TimedPosition> filtered{
      timedPos(2, 3, 4),
      timedPos(6, 7, 2)
    };

  LocalGpsFilterResults results{
    GeographicReference(GeographicPosition<double>(34.4*deg, 344.3*deg)),
    raw, filtered
  };

  auto motions = results.getGpsMotions(
      Duration<double>::minutes(3.0));
  EXPECT_EQ(motions.size(), 1);
  auto m = motions[0];

  EXPECT_NEAR((m.time - timeStamp(4.0)).seconds(), 0.0, 1.0e-6);
  EXPECT_NEAR(m.value[0].metersPerSecond(), 1.0, 1.0e-6);
  EXPECT_NEAR(m.value[1].metersPerSecond(), -0.5, 1.0e-6);
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

  TimedSampleCollection<GeographicPosition<double>> filteredPositions(
      filtered0.positions);


  int filteredCounter = 0;
  int corrCounter = 0;
  int marg = 3;
  for (int i = 0; i < n; i++) {
    TimedValue<GeographicPosition<double>> bad = corruptedPositions[i];
    TimedValue<GeographicPosition<double>> good = originalPositions[i];
    TimeStamp t = good.time;

    Optional<TimedValue<GeographicPosition<double>>> filteredOpt =
      filteredPositions.nearestTimedValue(t);

    if (!filteredOpt.defined()) {
      continue;
    }
    GeographicPosition<double> filtered = filteredOpt.get().value;

    Length<> distToGood = distance(filtered, good.value);
    //Length<> distToBad = distance(filtered, bad.value);

    corrCounter++;

    //EXPECT_LT(distToGood, distToBad); // TODO: Actually quite weak test, but better than nothing.

    EXPECT_LT(distToGood.meters(), 30);
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
      EXPECT_EQ((Array<Array<TimedValue<int>>>{values}),
          applySplits(values, times));
  }{
    Array<TimeStamp> times{lt(-0.3)};
    Array<TimedValue<int> > values{tvi(0.1, 3), tvi(0.2, 4)};
    EXPECT_EQ((Array<Array<TimedValue<int>>>{emptyArray, values}),
        applySplits(values, times));
  }{
    Array<TimeStamp> times{lt(0.9)};
    Array<TimedValue<int> > values{tvi(0.1, 3), tvi(0.2, 4)};
    EXPECT_EQ((Array<Array<TimedValue<int>>>{values, emptyArray}),
        applySplits(values, times));
  }{
    Array<TimeStamp> times{lt(0.15)};
    Array<TimedValue<int> > values{tvi(0.1, 3), tvi(0.2, 4)};
    EXPECT_EQ((Array<Array<TimedValue<int>>>{
      Array<TimedValue<int>>{tvi(0.1, 3)},
      Array<TimedValue<int>>{tvi(0.2, 4)},
    }),
    applySplits(values, times));
  }
}
