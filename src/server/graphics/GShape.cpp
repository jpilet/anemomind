/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GShape.h"
#include <server/common/math.h>

namespace sail {

GSettings::GSettings() {
  colormap = Arrayi(4);
  colormap[0] = 0;
  colormap[1] = 0;
  colormap[2] = 1;
  colormap[3] = 2;
}



int GShape::operator() (double x, double y) const {
  if (inside(x, y)) {
    double k = _settings.width + _settings.space;
    int index = int(floor(x/k));
    if (x - k*index <= _settings.width) {
      return index;
    }
    return -1;
  }
  return -1;
}

} /* namespace mmm */
