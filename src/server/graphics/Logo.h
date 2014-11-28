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

  // Evaluate the displacement of x.
  template <typename T>
  T eval(T y) const {
    return _a*sqr(y) + _b*y + _c;
  }

  template <typename T>
  T evalDeriv(T y) const {
    return 2*_a*y + _b;
  }

  Bender operator+ (const Bender &other) const {
    return Bender(_a + other._a, _b + other._b, _c + other._c);
  }


  double findClosestY(double x, double y, int iters = 12) const {
    return findClosestYReversed(y, x, iters);
  }

  double findClosestYReversed(double x, double y, int iters = 12) const;
 private:
  void iterateClosestReversed(double *x, double xDst, double yDst) const;
  double _a, _b, _c;
};


class Logo : public R2ImageRGB {
 public:

  class Colors {
   public:
    Colors();
    Logo::Vec bg, fg, magenta, max;
  };


  Logo(ASettings as = ASettings(),
       GSettings gs = GSettings());
  int getColorCode(double x, double y) const;

  Vec operator() (double x, double y) const;

  double width() const {return 2;}
  double height() const {return 2;}
 private:
  Colors _colors;
  Bender _bender, _aCurve;
  LineKM _aLine;
  AShape _A;
  GShape _G;
};

}

#endif /* LOGO_H_ */
