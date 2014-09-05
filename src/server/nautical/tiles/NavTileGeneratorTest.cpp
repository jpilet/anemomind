
#include <server/nautical/tiles/NavTileGenerator.h>

#include <gtest/gtest.h>

namespace sail {

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
  Array<Array<Nav>> result = generateTiles(
      tile, // A quarter of the world
      navs, 5);

  EXPECT_EQ(1, result.size());
  EXPECT_EQ(5, result[0].size());

  // Every point should be in the tile.
  for (const Nav& nav : result[0]) {
    EXPECT_TRUE(tile.contains(nav.geographicPosition()));
  }

  for (const Nav& nav : navs) {
    if (nav.time() < result[0].first().time()
        || nav.time() > result[0].last().time()) {
      EXPECT_FALSE(tile.contains(nav.geographicPosition()));
    }
  }
}


TEST(NavTileGenerator, SplitTest) {
  Array<Nav> navs(32);
  for (int i = 0; i < navs.size(); ++i) {
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
  Array<Array<Nav>> result = generateTiles(
      tile, // A quarter of the world
      navs, 5);

  EXPECT_EQ(2, result.size());
  EXPECT_EQ(5, result[0].size());
  EXPECT_EQ(5, result[1].size());
}


}  // namespace sail
