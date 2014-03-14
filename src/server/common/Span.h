/*
 * Span.h
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#ifndef SPAN_H_
#define SPAN_H_

#include <iosfwd>
#include "Array.h"

namespace sail {

class Span {
 public:
  Span() : _minv(1), _maxv(-1) {}
  bool initialized() const {
    return _minv <= _maxv;
  }

  Span(double value) : _minv(value), _maxv(value) {}
  Span(double a, double b);

  Span(Arrayd values) : _minv(1), _maxv(-1) {
    int count = values.size();
    for (int i = 0; i < count; i++) {
      extend(values[i]);
    }
  }


  double getMinv() {
    return _minv;
  }
  double getMaxv() {
    return _maxv;
  }

  void extend(const Span &other);
  void extend(double value);

  bool contains(double value) {
    return _minv <= value && value < _maxv;
  }

  virtual ~Span() {}

  double subtractMean();
 private:
  double _minv, _maxv;
};

std::ostream &operator<<(std::ostream &s, Span x);

} /* namespace sail */

#endif /* SPAN_H_ */
