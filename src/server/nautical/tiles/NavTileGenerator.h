#ifndef NAUTICAL_TILES_NAVTILEGENERATOR_H
#define NAUTICAL_TILES_NAVTILEGENERATOR_H

#include <server/common/Array.h>
#include <server/nautical/GeographicPosition.h>
#include <server/nautical/Nav.h>

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
 private:
  int _scale;
  int _x;
  int _y;
};

Array<Array<Nav>> generateTiles(TileKey tileKey,
                                const Array<Nav>& nav,
                                int maxNumNavs);

}  // namespace sail

#endif  // NAUTICAL_TILES_NAVTILEGENERATOR_H
