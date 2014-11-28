/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Logo.h"
#include <server/math/ADFunction.h>
#include <server/common/logging.h>

namespace sail {

Bender::Bender(double x[3]) {
  arma::mat A(3, 3);
  arma::mat B(3, 1);
  double y[3] = {0, 0.5, 1.0};
  for (int i = 0; i < 3; i++) {
    A(i, 0) = sqr(x[i]);
    A(i, 1) = x[i];
    A(i, 2) = 1.0;
    B(i, 0) = y[i];
  }
  arma::mat X = solve(A, B);
  _a = X(0, 0);
  _b = X(1, 0);
  _c = X(2, 0);
}

double Bender::findClosestYReversed(double x, double y, int iters) const {
  double xSol = x;
  for (int i = 0; i < iters; i++) {
    iterateClosestReversed(&xSol, x, y);
  }
  return xSol;
}

void Bender::iterateClosestReversed(double *x, double xDst, double yDst) const {
  arma::vec2 tangent = normalize(arma::vec{1.0, evalDeriv(*x)});
  double step = arma::dot(tangent, arma::vec2{xDst - *x, yDst - eval(*x)});
  *x += step*tangent[0];
}


Logo::Colors::Colors() {
  double f = 1.0/255;
  double g = 0.5;

  bg = Vec{1, 1, 1};
  fg = Vec{g, g, g};
  magenta = Vec{f*212, 0, f*87};
  max = Vec{0, 0, 0};
}

Logo::Logo(Settings s) : _A(s.A), _G(s.G),
           _aLine(s.A.makeALine()) {
  _bender = Bender(-_aLine.getM(), -_aLine.getK(), 0.0);
  _aCurve = _bender + Bender(0.0, _aLine.getK(), _aLine.getM());
  _width = 2;
  _height = 2;
}

int Logo::getColorCode(double xDist, double yDist) const {
  double xUndist = xDist - _bender.eval(yDist);
  if (xUndist <= _aLine(yDist)) {
    return (_A(xUndist, yDist)? 1 : -1);
  } else {
    double yClosest = _aCurve.findClosestY(xDist, yDist);
    double xClosest = _aCurve.eval(yClosest);

    double w = _G.width();
    double xLocal = w*yClosest;
    double yLocal = w*sqrt(sqr(xClosest - xDist) + sqr(yClosest - yDist));
    int gIndex = _G(xLocal, yLocal);
    if (gIndex == -1) {
      return -1;
    } else {
      return _G.settings().colormap[gIndex];
    }
  }
}

Logo::Vec Logo::operator() (double x, double y) const {
  int index = getColorCode(x - 0.5*_A.width(), _height - y);
  switch (index) {
   case -1:
     return _colors.bg;
   case 0:
     return _colors.fg;
   case 1:
     return _colors.magenta;
   case 2:
     return _colors.max;
   default:
     LOG(FATAL) << "Illegal color code";
  };
  return Logo::Vec();
}

Logo::Settings makeLogoSettings1() {
  return Logo::Settings();
}

Logo::Settings makeLogoSettings2() {
  Logo::Settings s;
  s.A.hasWaist = false;
  s.A.strokeWidth = 0.0;
  s.G.margin = s.G.space;
  s.G.height = 1.0/goldenRatio();
  return s;
}

Logo::Settings makeLogoSettings3() {
  Logo::Settings s;
  s.A.filled = false;
  s.A.strokeWidth = 0.18;
  s.A.waistHeight = 0.12;
  return s;
}

Logo::Settings makeLogoSettings4() {
  Logo::Settings s = makeLogoSettings3();
  s.G.margin = s.G.space;
  return s;
}

Logo::Settings makeLogoSettings5() {
  Logo::Settings s = makeLogoSettings3();
  s.A.hasWaist = false;
  return s;
}

Logo::Settings makeLogoSettings6() {
  Logo::Settings s = makeLogoSettings5();
  s.G.margin = s.G.space;
  return s;
}

namespace {
  Arrayi makeColormap6() {
    Arrayi m(6);
    m[0] = 0;
    m[1] = 0;
    m[2] = 0;
    m[3] = 1;
    m[4] = 1;
    m[5] = 2;
    return m;
  }
}

Logo::Settings makeLogoSettings7() {
  Logo::Settings s = makeLogoSettings2();
  s.G.colormap = makeColormap6();
  s.G.height = 1.0;
  return s;
}

Logo::Settings makeLogoSettings8() {
  Logo::Settings s = makeLogoSettings2();
  s.G.height = 1.0;
  return s;
}





} /* namespace mmm */
