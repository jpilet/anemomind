/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GSHAPE_H_
#define GSHAPE_H_

namespace sail {

class GSettings {
 public:
  int count = 4;
  double width = 1.0;
  double space = 0.2;
  double margin = 0.0;
};

class GShape {
 public:
  GShape(GSettings settings) : _settings(settings) {}

  int operator() (double x, double y) const;

  double height() const {return 1.0 + _settings.margin;}

  double width() const {
    return _settings.width*_settings.count
        + (_settings.count - 1)*_settings.space;
  }

  bool inside(double x, double y) const {
    if (0 <= _settings.margin && y <= height()) {
      return 0 <= x && x <= width();
    }
    return false;
  }

  int count() const {
    return _settings.count;
  }
 private:
  GSettings _settings;
};

}

#endif /* GSHAPE_H_ */
