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
#include <server/plot/GnuPlotModel.h>
#include <server/nautical/Visualize.h>


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

  GpsFilterResults results{
    GeographicReference(GeographicPosition<double>(34.4*deg, 344.3*deg)),
    raw, filtered
  };

  auto motions = results.getGpsMotions();
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

  auto corrupted = original.overrideChannels(
      srcName,
      {{GPS_POS, makeDispatchDataFromSamples<GPS_POS>(
          srcName, corruptedPositions)}});

  // This test case is broken. An assertion is fired in debug mode.
  // It is fixed by https://github.com/jpilet/anemomind/pull/698
  auto filtered0 = filterGpsData(corrupted);

  EXPECT_FALSE(filtered0.filteredLocalPositions.empty());

  auto filteredPositions = filtered0.getGlobalPositions();



  bool visualize = false;
  if (visualize) {
    GnuPlotModel model(2);

    makeTrajectoryPlot(filtered0.geoRef,
        TimedSampleCollection<GeographicPosition<double>>(filteredPositions))->render(&model);

    makeTrajectoryPlot(filtered0.geoRef,
            originalPositions)->render(&model);

    model.show();
  }


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

