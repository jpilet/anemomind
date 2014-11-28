/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ASHAPE_H_
#define ASHAPE_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/graphics/R2Image.h>
#include <armadillo>

namespace sail {

inline arma::vec2 normalize(const arma::vec2 &x) {
  double len = arma::norm(x, 2);
  return (1.0/len)*x;
}

class ASettings {
 public:
  double width = 0.7;
  double height = 2.0;
  bool filled = true;
  bool hasWaist;
  double strokeWidth = 0.2;
  double waistHeight = 0.3;

  LineKM makeALine() const {
    return LineKM(0, 1, width/2, 0);
  }
};

class AShape  {
 public:
  AShape(ASettings settings);

  bool operator() (double x, double y) const;


  double leftEdge(double x, double y) const {
    return -_bdfun(y);
  }
  double rightEdge(double x, double y) const {
    return _bdfun(y);
  }

  bool inside(double x, double y) const {
    return 0 <= y && y <= 1 && leftEdge(x, y) <= x && x <= rightEdge(x, y);
  }

  bool onBorder(double x, double y) const;

  bool onWaist(double x, double y) const {
    return _settings.waistHeight <= y && y <= _settings.waistHeight + _settings.strokeWidth;
  }

  double width() const {
    return _settings.width;
  }
 private:
  LineKM _bdfun;
  ASettings _settings;

  arma::vec2 _top, _leftNormal, _rightNormal, _bottomRight;
};

} /* namespace mmm */

#endif /* ASHAPE_H_ */
