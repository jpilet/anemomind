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
  std::cout << EXPR_AND_VAL_AS_STRING(navs.size()) << std::endl;
  return navs.sliceFrom(3500);
}

// Check that the filtered signal is reasonbly close to the non-filtered one.
TEST(GpsFilterTest, PsarosTest) {
  auto navs = getPsarosTestData();
  GpsFilter::Settings settings;
  auto results = GpsFilter::filter(navs, settings);

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
