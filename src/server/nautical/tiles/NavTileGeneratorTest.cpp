
#include <server/nautical/tiles/NavTileGenerator.h>
#include <server/nautical/tiles/TileUtils.h>
#include <server/nautical/tiles/NavTileUploader.h>

#include <gtest/gtest.h>
#include <iostream>

namespace sail {

using namespace NavCompat;
using namespace std;

TEST(NavTileGenerator, SmokeTest) {
  Array<Nav> navs(100);

  TimeStamp now = TimeStamp::now();

  // Generate a fake curve
  for (int i = 0; i < navs.size(); ++i) {
    double t = double(i) / 100.0;
    navs[i].setGeographicPosition(
        GeographicPosition<double>(
            Angle<double>::degrees((t * 10 - 5)),
            Angle<double>::degrees(t * t * 20 - 10)));
    navs[i].setTime(now + Duration<double>::seconds(i));
  }

  TileKey tile(1, 1, 0);
  map<TileKey, vector<int>> tileIndex = tilesForNav(navs, 2);
  Array<Array<Nav>> result = generateTiles(
      tile, // A quarter of the world
      navs,
      tileIndex[tile],
      5, 1.0_minutes);

  EXPECT_EQ(1, result.size());
  EXPECT_EQ(5, result[0].size());

  // Every point should be in the tile.
  for (const Nav& nav : result[0]) {
    EXPECT_TRUE(tile.contains(nav.geographicPosition()));
  }

  for (int i = 0; i < navs.size(); ++i) {
    const Nav& nav = navs[i];
    if (nav.time() < result[0].first().time()
        || nav.time() > result[0].last().time()) {
      EXPECT_FALSE(tile.contains(nav.geographicPosition()))
        << "at nav[" << i << "]";
    }
  }
}


TEST(NavTileGenerator, SplitTest) {
  Array<Nav> navs(32);
  auto start = TimeStamp::UTC(2016, 02, 19, 16, 23, 0);
  for (int i = 0; i < navs.size(); ++i) {
    navs[i].setTime(start + Duration<double>::seconds(i));
    navs[i].setGeographicPosition(
        GeographicPosition<double>(
            Angle<double>::degrees((i/32.0 * 10 + 1)),
            Angle<double>::degrees(10)));
  }
  // Suddenly a few points go to the south emisphere.
  for (int i = 14; i < 17; ++i) {
    navs[i].setGeographicPosition(
        GeographicPosition<double>(
            navs[i].geographicPosition().lon(),
            Angle<double>::degrees(-10)));
  }

  TileKey tile(1, 1, 0);
  map<TileKey, vector<int>> tileIndex = tilesForNav(navs, 2);
  Array<Array<Nav>> result = generateTiles(
      tile, // A quarter of the world
      navs,
      tileIndex[tile],
      5, 1.0_minutes);

  EXPECT_EQ(2, result.size());
  EXPECT_EQ(5, result[0].size());
  EXPECT_EQ(5, result[1].size());
}

TEST(NavTileGenerator, TileKeyTest) {
  for (int i = 0; i < 10; ++i) {
    GeographicPosition<double> a(
        Angle<double>::degrees((i/32.0 * 10 + 1)),
        Angle<double>::degrees(i*3341 %180));
    GeographicPosition<double> b(
        a.lon() + Angle<double>::degrees(360 * i),
        a.lat() - Angle<double>::degrees(360 * i));
    EXPECT_EQ(TileKey::fromPos(i, a).stringKey(),
              TileKey::fromPos(i, b).stringKey());
  }
}

TEST(NavTileGenerator, TileCoordTest) {
  GeographicPosition<double> lizard(
      Angle<>::degrees(-5.206271),
      Angle<>::degrees(49.958910));
  EXPECT_NEAR(0.48553780374881694 , posToTileX(0, lizard), 1e-6);
  EXPECT_NEAR(0.33932217262515935, posToTileY(0, lizard), 1e-6);
}

}  // namespace sail
