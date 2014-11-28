/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef LOGO_H_
#define LOGO_H_

#include <server/graphics/AShape.h>
#include <server/graphics/GShape.h>
#include <cmath>

namespace sail {

class Bender {
 public:
  Bender() : _a(NAN), _b(NAN), _c(NAN) {}
  Bender(double a, double b, double c) :
    _a(a), _b(b), _c(c) {}
  Bender(double x[3]);
  double remapX(double y) const {
    return _a*sqr(y) + _b*y + _c;
  }
 private:
  double _a, _b, _c;
};

class Logo {
 public:
  Logo(ASettings as = ASettings(),
       GSettings gs = GSettings());
 private:
  Bender _bender;
  LineKM _aLine;
  AShape _A;
  GShape _G;
};

}

#endif /* LOGO_H_ */
