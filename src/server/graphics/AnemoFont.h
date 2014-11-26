/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ANEMOFONT_H_
#define ANEMOFONT_H_

#include <cmath>
#include <server/common/math.h>
#include <memory>
#include <server/common/Span.h>
#include <server/graphics/R2Image.h>

namespace sail {
namespace anemofont {

class Settings {
 public:
  double strokeSize = 0.1;
  double middleBarSize = 0.1;
  double radius = 2.0;
  double letterSpacing = 0.1;

  double arcfun(double y, bool left) const {
    double sol1, sol2;
    solveQuadratic(-2*radius, sqr(y - 0.5), &sol1, &sol2);
    return (left? -sol1 : sol1);
  }
};

class Primitive {
 public:
  typedef std::shared_ptr<Primitive> Ptr;
  virtual bool inside(double x, double y) const = 0;
  virtual double leftMost() const = 0;
  virtual double rightMost() const = 0;
  virtual double middle() const = 0;
  virtual ~Primitive() {}
};

class Arc : public Primitive {
 public:
  Arc(bool upper, bool left, const Settings &s) :
    _upper(upper), _left(left), _s(s) {}

  bool inside(double x, double y) const {
    if (_upper == (y < 0.5)) {
      double refx = _s.arcfun(y, _left);
      return std::abs(x - refx) < _s.strokeSize;
    }
    return false;
  }

  double leftMost() const {
    return std::min(0.0, _s.arcfun(0, _left)) - _s.strokeSize;
  }

  double rightMost() const {
    return std::max(0.0, _s.arcfun(0, _left)) + _s.strokeSize;
  }

  double middle() const {
    return 0;
  }
 private:
  Settings _s;
  bool _upper, _left;
};

class Vert : public Primitive {
 public:
  Vert(const Settings &s) : _s(s) {}

  bool inside(double x, double y) const {
    return std::abs(x) < _s.strokeSize;
  }

  double leftMost() const {
    return _s.strokeSize;
  }

  double rightMost() const {
    return _s.strokeSize;
  }

  double middle() const {
    return 0.0;
  }
 private:
  Settings _s;
};

class MiddleBar : public Primitive {
 public:
  MiddleBar(Spand xspan, const Settings &s) : _s(s),
    _xspan(xspan) {}
  bool inside(double x, double y) const {
    return _xspan.contains(x) && std::abs(y) < _s.middleBarSize;
  }

  double leftMost() const {
    return _xspan.minv();
  }

  double rightMost() const {
    return _xspan.maxv();
  }

  double middle() const {
    return NAN;
  }
 private:
  Settings _s;
  Spand _xspan;
};

class Shift : public Primitive {
 public:
  Shift(Primitive::Ptr x, double shift) : _x(x), _shift(shift) {}

  double leftMost() const {
    return _x->leftMost() + _shift;
  }

  double rightMost() const {
    return _x->leftMost() + _shift;
  }

  double middle() const {
    return _x->middle() + _shift;
  }

  bool inside(double x, double y) const {
    return _x->inside(x - _shift, y);
  }
 private:
  Primitive::Ptr _x;
  double _shift;
};

class Renderer : public R2ImageRGB {
 public:
  Renderer();
  void write(char c); // any of the letters in "anemomind"
 private:
  double _left;
  std::vector<Primitive::Ptr> _primitives;
};

}
}

#endif /* ANEMOFONT_H_ */
