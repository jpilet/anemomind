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
  // How wide the calligraphic pen is
  double strokeSize = 0.12;

  // How thick the horizontal lines in a and e are
  double middleBarSize = 0.05;

  // The radius of the circular arcs in letters such as 'n'
  double radius = 0.55; // at least 0.5

  // The radius of a more compact circular arc used for 'm' and 'o'
  double compactRadius = 0.77;

  // Basic amount of whitespace between letters
  double letterSpacing = 0.05;

  // Extra amount of whitespace added at vertical lines
  // (because rounded lines are already surrounded with lots of white space)
  double straightPadding = 0.05;

  // How connected the primitives are when they meet
  double intersect = 0.10;

  double arcfun(double y, bool left, bool compact) const {
    double sol1, sol2;
    double r = (compact? compactRadius : radius);
    solveQuadratic(-2*r, sqr(y - 0.5), &sol1, &sol2);
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
  Arc(bool upper, bool left, const Settings &s, bool compact = false) :
    _upper(upper), _left(left), _compact(compact), _s(s) {}

  bool inside(double x, double y) const {
    if (_upper == (y <= 0.5)) {
      double refx = _s.arcfun(y, _left, _compact);
      return std::abs(x - refx) <= _s.strokeSize;
    }
    return false;
  }

  double leftMost() const {
    return std::min(0.0, _s.arcfun(0, _left, _compact)) - _s.strokeSize;
  }

  double rightMost() const {
    return std::max(0.0, _s.arcfun(0, _left, _compact)) + _s.strokeSize;
  }

  double middle() const {
    return 0;
  }
 private:
  bool _compact;
  Settings _s;
  bool _upper, _left;
};

class Vert : public Primitive {
 public:
  Vert(const Settings &s, Spand yspan = Spand(0, 1)) : _s(s), _yspan(yspan) {}

  bool inside(double x, double y) const {
    return std::abs(x) <= _s.strokeSize && _yspan.contains(y);
  }

  double leftMost() const {
    return -_s.strokeSize;
  }

  double rightMost() const {
    return _s.strokeSize;
  }

  double middle() const {
    return 0.0;
  }
 private:
  Settings _s;
  Spand _yspan;
};

class MiddleBar : public Primitive {
 public:
  MiddleBar(Spand xspan, const Settings &s) : _s(s),
    _xspan(xspan) {}
  bool inside(double x, double y) const {
    return _xspan.contains(x) && std::abs(y - 0.5) < _s.middleBarSize;
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

  static Shift *middleAt(Primitive *x, double at) {
    return new Shift(Primitive::Ptr(x), at - x->middle());
  }

  static Shift *leftAt(Primitive *x, double at) {
    return new Shift(Primitive::Ptr(x), at - x->leftMost());
  }

  double leftMost() const {
    return _x->leftMost() + _shift;
  }

  double rightMost() const {
    return _x->rightMost() + _shift;
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

  Vec operator() (double x, double y) const;

  double width() const {
    return _left;
  }

  double height() const {
    return 1.0;
  }
 private:
  double _left;
  std::vector<Primitive::Ptr> _primitives;
  void add(Primitive *x) {
    _primitives.push_back(Primitive::Ptr(x));
    _left = std::max(_left, x->rightMost());
  }
};

}
}

#endif /* ANEMOFONT_H_ */
