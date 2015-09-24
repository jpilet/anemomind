/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/GpsFilter.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/plot/extra.h>


using namespace sail;

MDArray2d getRawPositions(GpsFilter::Results r, Array<Nav> navs) {
  int n = navs.size();
  MDArray2d X(n, 2);
  for (int i = 0; i < n; i++) {
    auto pos = r.geoRef.map(navs[i].geographicPosition());
    X(i, 0) = pos[0].meters();
    X(i, 1) = pos[1].meters();
  }
  return X;
}

Array<Nav> getPsarosTestData() {
  auto p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("psaros33_Banque_Sturdza")
    .pushDirectory("2014")
    .pushDirectory("20140821").get();
  auto navs = scanNmeaFolder(p, Nav::debuggingBoatId());
  return navs.sliceFrom(3500);
}

Array<Nav> applyOutliers(Array<Nav> navs) {
  int from = int(floor(navs.size()*0.05));
  int to = int(floor(navs.size()*0.15));
  Array<Nav> dst = navs.dup();
  Angle<double> offset = Angle<double>::degrees(0.5/60); // half a minute.
  for (int i = 0; i < navs.size(); i++) {
    if (i % 10 == 0) {
      auto &x = dst[i];
      auto g = x.geographicPosition();
      auto g2 = GeographicPosition<double>(g.lon(), g.lat() + offset);
      x.setGeographicPosition(g2);
    }
  }
  return dst;
}



void runPsarosTest(Array<Nav> navs, Array<Nav> navsToFilter) {
  GpsFilter::Settings settings;
  if (navsToFilter.empty()) {
    navsToFilter = navs;
  }

  auto results = GpsFilter::filter(navsToFilter, settings);
  auto filtered = results.filteredNavs();
  EXPECT_EQ(filtered.size(), navs.size());

  auto reasonableMotionCount = 0;
  auto reasonablePositionCount = 0;
  for (int i = 0; i < navs.size(); i++) {
    auto a = filtered[i];
    auto b = navs[i];
    auto motionDif = a.gpsMotion() - b.gpsMotion();
    auto motionDifNormKnots = sqrt(sqr(motionDif[0].knots()) + sqr(motionDif[1].knots()));
    if (motionDifNormKnots < 1.0) {
      reasonableMotionCount++;
    }

    auto posDif = results.geoRef.map(a.geographicPosition()) -
        results.geoRef.map(b.geographicPosition());
    auto posDifNormMeters = sqrt(sqr(posDif[0].meters()) + sqr(posDif[1].meters()));
    if (posDifNormMeters < 3) {
      reasonablePositionCount++;
    }
  }
  auto minCount = 0.8*navs.size();
  EXPECT_LT(minCount, reasonableMotionCount);
  EXPECT_LT(minCount, reasonablePositionCount);

  bool visualize = true;
  if (visualize) {
    GnuplotExtra plot;
    plot.set_style("lines");
    plot.plot(results.Xmeters);
    plot.set_style("points");
    plot.plot(getRawPositions(results, navsToFilter));
    plot.show();
  }
}

// Check that the filtered signal is reasonbly close to the non-filtered one.
TEST(GpsFilterTest, PsarosTest) {
  auto navs = getPsarosTestData();

  runPsarosTest(navs, Array<Nav>());
  runPsarosTest(navs, applyOutliers(navs));

}
