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
#include <server/math/nonlinear/Ceres1dSolver.h>
#include <server/common/Span.h>


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

// Check that the filtered signal is reasonbly close to the non-filtered one.
TEST(GpsFilterTest, PsarosTest) {
  auto navs = getPsarosTestData();
  GpsFilter::Settings settings;


  settings.useCeres = true;
  if (settings.useCeres) {


    EXPECT_NEAR(Ceres1dSolver::softSqrt(3.0001, 3.0),
      Ceres1dSolver::softSqrt(2.9999, 3.0), 0.001);
  }



  settings.filterSettings.iters = 60;
  settings.filterSettings.lambda = 0.1;

  auto results = GpsFilter::filter(navs, settings);
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
    if (posDifNormMeters < 10) {
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
    plot.plot(getRawPositions(results, navs));
    plot.show();
  }
}

Array<Nav> getAllPsarosData() {
  auto p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("psaros33_Banque_Sturdza")
    .pushDirectory("2014").get();
  auto navs = scanNmeaFolder(p, Nav::debuggingBoatId());
  std::sort(navs.begin(), navs.end());
  return navs;
}








