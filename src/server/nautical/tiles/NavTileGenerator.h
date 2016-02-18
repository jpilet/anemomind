#ifndef NAUTICAL_TILES_NAVTILEGENERATOR_H
#define NAUTICAL_TILES_NAVTILEGENERATOR_H

#include <server/common/Array.h>
#include <server/nautical/GeographicPosition.h>
#include <server/nautical/Nav.h>
#include <set>
#include <tuple>

namespace sail {

class TileKey {
 public:
  TileKey(int scale, int x, int y) : _scale(scale), _x(x), _y(y) { }

  static TileKey fromPos(int scale, const GeographicPosition<double>& pos);

  std::string stringKey() const;

  int scale() const { return _scale; }
  int x() const { return _x; }
  int y() const { return _y; }

  bool contains(const GeographicPosition<double>& pos) const {
    return fromPos(_scale, pos) == *this;
  }

  bool operator ==(const TileKey& other) const {
    return _scale == other._scale && _x == other._x && _y == other._y;
  }

  bool operator <(const TileKey& other) const {
    return std::make_tuple(_scale, _x, _y)
      < std::make_tuple(other._scale, other._x, other._y);
  }

 private:
  int _scale;
  int _x;
  int _y;
};

// Convert to openstreetmap projection.
double posToTileX(int scale, const GeographicPosition<double>& pos);
double posToTileY(int scale, const GeographicPosition<double>& pos);

Array<NavCollection> generateTiles(TileKey tileKey,
                                const NavCollection& nav,
                                int maxNumNavs);

// Return a set of tiles on which "navs" should appear.
std::set<TileKey> tilesForNav(const NavCollection& navs, int maxScale);

// Generate a unique identifier for this Nav curve.
std::string tileCurveId(std::string boatId, const NavCollection& navs);

}  // namespace sail

#endif  // NAUTICAL_TILES_NAVTILEGENERATOR_H
