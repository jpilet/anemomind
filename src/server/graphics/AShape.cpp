/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "AShape.h"

namespace sail {


AShape::AShape(ASettings settings) :
  _settings(settings), _bdfun(0, 1, settings.width/2.0, 0.0),
  _top{0, 1.0}, _leftNormal{1, settings.width/2},
  _rightNormal{-1, settings.width/2},
  _bottomRight{settings.width/2, 0.0} {
}

bool AShape::operator() (double x, double y) const {
  if (inside(x, y)) {
    if (_settings.filled) {
      if (onLeftBorder(x, y)) { // On the right border
        return false;
      } else { // On the inside
        if (_settings.hasWaist) {
          return !onWaist(x, y);
        }
        return true;
      }
    } else {
      if (onBorder(x, y)) {
        return true;
      } else {
        if (_settings.hasWaist) {
          return onWaist(x, y);
        }
        return false;
      }
    }
  }
  return false;
}

bool AShape::onLeftBorder(double x, double y) const {
  arma::vec2 X{x, y};
  return std::abs(arma::dot(X - _top, _leftNormal)) <= _settings.strokeWidth;
}

bool AShape::onRightBorder(double x, double y) const {
  arma::vec2 X{x, y};
  return std::abs(arma::dot(X - _top, _rightNormal)) <= _settings.strokeWidth;
}


bool AShape::onBorder(double x, double y) const {
  return onLeftBorder(x, y) || onRightBorder(x, y);
}

}
