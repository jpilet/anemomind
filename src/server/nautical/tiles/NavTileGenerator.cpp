#include <server/nautical/tiles/NavTileGenerator.h>

#include <cmath>
#include <server/common/ArrayBuilder.h>
#include <server/common/string.h>
#include <server/math/geometry/SimplifyCurve.h>
#include <server/common/Functional.h>

namespace sail {

using namespace NavCompat;

namespace {

Array<Nav> makeTileElement(TileKey tileKey,
                           const Array<Nav>& navs,
                           int maxNumNavs) {
  if (navs.size() <= maxNumNavs) {
    return navs;
  }

  CurveSimplifier curve(false);
  for (const Nav& nav : navs) {
    curve.addPoint(
       posToTileX(0, nav.geographicPosition()), 
       posToTileY(0, nav.geographicPosition()));
  }
  std::vector<int> priorities = curve.priorities();

  ArrayBuilder<Nav> result;
  for (int i = 0; i < navs.size(); ++i) {
    if (priorities[i] < maxNumNavs) {
      result.add(navs[i]);
    }
  }
  return result.get();
}

} // namespace

// Inspired by
// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#C.2FC.2B.2B
double posToTileX(int scale, const GeographicPosition<double>& pos) {
  double scaleFactor = double(long(1) << scale);
  return (pos.lon().normalizedAt0().degrees() + 180.0) / 360.0 * scaleFactor;
}

double posToTileY(int scale, const GeographicPosition<double>& pos) {
  double scaleFactor = double(long(1) << scale);
  return (1.0 - log( tan(pos.lat()) + 1.0 / cos(pos.lat())) / M_PI)
                 / 2.0 * scaleFactor; 
}


TileKey TileKey::fromPos(int scale, const GeographicPosition<double>& pos) {
  return TileKey(scale,
                 (int)(floor(posToTileX(scale, pos))),
                 (int)(floor(posToTileY(scale, pos))));
}

std::string TileKey::stringKey() const {
  return stringFormat("s%dx%dy%d", _scale, _x, _y);
}

Array<Array<Nav>> generateTiles(TileKey tileKey,
                                const Array<Nav>& navs,
                                const std::vector<int>& navIndices,
                                int maxNumNavs,
                                Duration<> curveCutThreshold) {
  ArrayBuilder<Array<Nav>> result;

  // The curve might enter and leave the tile multiple times.
  // Group together consecutive points that are in the tile.
  for (int i = 0; i < navIndices.size(); /*The missing inc here is not a bug!*/) {
    int first = navIndices[i];
    int end = first;
    for (int j = i; (j + 1) < navIndices.size(); ++j) {
      int a = navIndices[j];
      int b = navIndices[j + 1];
      if ((b != (a + 1))
          || (navs[b].time() - navs[a].time()) > curveCutThreshold) {
        break;
      }
      end = b;
      i = j + 1;
    }

    // The loop before sets "end" to the last nav to be included.
    // slice expects
    ++end;

    if (end > first) {
      result.add(makeTileElement(tileKey, navs.slice(first, end), maxNumNavs));
    }
 
    // i is now the last processed index. Move the next index to process.
    ++i;
  }
  return result.get();
}

std::map<TileKey, std::vector<int>> tilesForNav(
    const Array<Nav>& navs, int maxScale) {
  std::map<TileKey, std::vector<int>> result;
  for (int i = 0; i < navs.size(); ++i) {
    const Nav& nav = navs[i];
    for (int scale = 0; scale < maxScale; scale++) {
      TileKey key = TileKey::fromPos(scale, nav.geographicPosition());

      std::map<TileKey, std::vector<int>>::iterator it = result.find(key);

      if (it != result.end()) {
        it->second.push_back(i);
      } else {
        result[key] = std::vector<int>{i};
      }
    }
  }
  return result;
}

std::string tileCurveId(std::string boatId, const NavDataset& navs) {
  // TODO: hash this string.
  return boatId + getFirst(navs).time().toString() + getLast(navs).time().toString();
}

}  // namespace sail
