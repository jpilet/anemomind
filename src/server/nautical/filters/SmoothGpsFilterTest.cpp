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
  *((unsigned long *)nullptr) = 0xDEADBEEF;


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

}

TEST(SmoothGpsFilterTest, TestIt) {
  auto original = getPsarosTestData();
  EXPECT_FALSE(original.isDefaultConstructed());
  auto originalPositions = original.samples<GPS_POS>();
  auto corruptedPositions = applyOutliers(originalPositions);

  EXPECT_EQ(originalPositions.size(), corruptedPositions.size());
  int n = originalPositions.size();

  auto corrupted = original.dup();
  EXPECT_FALSE(corrupted.isDefaultConstructed());
  corrupted.setMerged(GPS_POS, makeDispatchDataFromSamples<GPS_POS>("corrupted", corruptedPositions));

  auto filtered0 = filterGpsData(corrupted);

  EXPECT_FALSE(filtered0.localPositions.empty());

  auto filteredPositions = filtered0.getGlobalPositions();

  int filteredCounter = 0;
  int corrCounter = 0;
  double maxDistance = 0.0;
  int marg = 3;
  for (int i = 0; i < n; i++) {
    if (i % 10 == 0) {
      auto good = originalPositions[i];
      auto bad = corruptedPositions[i];
      auto t = good.time;

      // Step to the next filtered sample that is close to our good/bad ones.
      while (filteredCounter < filteredPositions.size() &&
          t < filteredPositions[filteredCounter].time) {
        filteredCounter++;
      }

      for (int j = std::max(0, filteredCounter - marg);
          j <= std::min(filteredPositions.size()-1, filteredCounter + marg); j++) {
        auto x = filteredPositions[j];
        auto distToGood = distance(x.value, good.value);
        auto distToBad = distance(x.value, bad.value);
        corrCounter++;
        EXPECT_LT(distToGood, distToBad); // TODO: Actually quite weak test, but better than nothing.
      }

      if (filteredCounter == filteredPositions.size()) {
        break;
      }
    }
  }
  EXPECT_LT(60, corrCounter);
}

