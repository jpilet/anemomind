/*
 * Span.cpp
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#include "Span.h"
#include <algorithm>
#include <iostream>

namespace sail {

void Span::extend(const Span &other) {
  if (other.initialized()) {
    if (initialized()) {
      _minv = std::min(_minv, other._minv);
      _maxv = std::max(_maxv, other._maxv);
    } else {
      _minv = other._minv;
      _maxv = other._maxv;
    }
  }
}

Span::Span(double a, double b) {
  if (a <= b) {
    _minv = a;
    _maxv = b;
  } else {
    _minv = b;
    _maxv = a;
  }
}

void Span::extend(double value) {
  if (initialized()) {
    _minv = std::min(_minv, value);
    _maxv = std::max(_maxv, value);
  } else {
    _minv = value;
    _maxv = value;
  }
}

double Span::subtractMean() {
  double mean = 0.5*(_minv + _maxv);
  _minv -= mean;
  _maxv -= mean;
  return mean;
}

Span Span::expand(double x) {
  assert(initialized());
  return Span(_minv - x, _maxv + x);
}

std::ostream &operator<<(std::ostream &s, Span x) {
  s << "Span(" << x.getMinv() << ", " << x.getMaxv() << ")";
  return s;
}

} /* namespace sail */
