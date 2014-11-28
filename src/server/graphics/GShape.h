/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GSHAPE_H_
#define GSHAPE_H_

#include <server/common/math.h>
#include <server/common/Array.h>

namespace sail {

class GSettings {
 public:
  GSettings();
  Arrayi colormap;
  int count() const {
    return colormap.size();
  }
  double height = 1.0;
  double width = 1.0;
  double space = 0.2;
  double margin = 0.0;
};

class GShape {
 public:
  GShape(GSettings settings) : _settings(settings) {}

  int operator() (double x, double y) const;

  double height() const {return _settings.height + _settings.margin;}

  double width() const {
    return _settings.width*_settings.count()
        + (_settings.count() - 1)*_settings.space;
  }

  bool inside(double x, double y) const {
    if (_settings.margin <= y && y <= height()) {
      return 0 <= x && x <= width();
    }
    return false;
  }

  int count() const {
    return _settings.count();
  }

  const GSettings &settings() const {
    return _settings;
  }
 private:
  GSettings _settings;
};

}

#endif /* GSHAPE_H_ */
