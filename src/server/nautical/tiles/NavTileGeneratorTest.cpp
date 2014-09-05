
#include <server/nautical/tiles/NavTileGenerator.h>

#include <gtest/gtest.h>

namespace sail {

TEST(NavTileGenerator, SmokeTest) {
  Array<Nav> navs(100);

  // Generate a fake curve
  for (int i = 0; i < navs.size(); ++i) {
    double t = double(i) / 100.0;
    navs[i].setGeographicPosition(
        GeographicPosition<double>(
            Angle<double>::degrees((t * 10 - 5)),
            Angle<double>::degrees(t * t * 20 - 10)));
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
}

}  // namespace sail
