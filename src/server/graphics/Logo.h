/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef LOGO_H_
#define LOGO_H_

#include <server/common/math.h>
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

  class Settings {
   public:
    ASettings A;
    GSettings G;
  };

  Logo(Settings s = Settings());
  int getColorCode(double x, double y) const;

  Vec operator() (double x, double y) const;

  double width() const {return _width;}
  double height() const {return _height;}

  void setSize(double w, double h) {
    _width = w;
    _height = h;
  }
 private:
  double _width;
  double _height;
  Colors _colors;
  Bender _bender, _aCurve;
  LineKM _aLine;
  AShape _A;
  GShape _G;
};

Logo::Settings makeLogoSettings1();
Logo::Settings makeLogoSettings2();
Logo::Settings makeLogoSettings3();
Logo::Settings makeLogoSettings4();
Logo::Settings makeLogoSettings5();
Logo::Settings makeLogoSettings6();
Logo::Settings makeLogoSettings7();
Logo::Settings makeLogoSettings8();
Array<Logo::Settings> makeLogoSettings();

class Combine : public R2ImageRGB {
 public:
  Combine(R2ImageRGB::Ptr icon, R2ImageRGB::Ptr text,
      bool mirror) : _icon(icon), _text(text), _mirror(mirror),
      _w0(_icon->width()), _hOffset(icon->height() - text->height()) {}

  double width() const {
    return _icon->width() + _text->width();
  }

  double height() const {
    return std::max(_icon->height(), _text->height());
  }

  Vec operator() (double x, double y) const {
    if (x < _w0) {
      return (*_icon)((_mirror? _w0 - x : x), y);
    } else {
      return (*_text)(x - _w0, y - _hOffset);
    }
  }
 private:
  R2ImageRGB::Ptr _icon, _text;
  bool _mirror;
  double _w0;
  double _hOffset;
};

class Padding : public R2ImageRGB {
 public:
  Padding(R2ImageRGB::Ptr im, double pd) : _im(im), _pd(pd) {}

  double width() const {
    return _im->width() + 2*_pd;
  }

  double height() const {
    return _im->height() + 2*_pd;
  }

  Vec operator() (double x, double y) const {
    return (*_im)(x - _pd, y - _pd);
  }

 private:
  R2ImageRGB::Ptr _im;
  double _pd;
};

}

#endif /* LOGO_H_ */
