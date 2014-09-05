#include <server/nautical/tiles/NavTileGenerator.h>

#include <cmath>
#include <server/common/ArrayBuilder.h>
#include <server/common/string.h>
#include <server/math/geometry/SimplifyCurve.h>

namespace sail {

namespace {

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

Array<Nav> makeTileElement(TileKey tileKey,
                           const Array<Nav>& navs,
                           int maxNumNavs) {
  if (navs.size() <= maxNumNavs) {
    return navs;
  }

  CurveSimplifier curve(false);
  for (const Nav& nav : navs) {
    curve.addPoint(
       posToTileX(tileKey.scale(), nav.geographicPosition()), 
       posToTileY(tileKey.scale(), nav.geographicPosition()));
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
                                int maxNumNavs) {
  Array<bool> inOrOut = navs.map<bool>(
      [&] (const Nav& nav) -> bool {
          return tileKey.contains(nav.geographicPosition());
      });
  ArrayBuilder<Array<Nav>> result;

  // The curve might enter and leave the tile multiple times.
  // Group together consecutive points that are in the tile.
  for (int i = 0; i < navs.size(); ) {
    int first = inOrOut.sliceFrom(i).find(true);
    if (first == -1) {
      break;  // nothing more in this tile.
    }
    first += i;

    int end = inOrOut.sliceFrom(first + i).find(false);
    if (end == -1) {
      end = inOrOut.size();
    } else {
      end += first + i;
    }

    if (end > first) {
      result.add(makeTileElement(tileKey, navs.slice(first, end), maxNumNavs));
    }
    i = end;
  }
  return result.get();
}

}  // namespace sail
