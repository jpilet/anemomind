/*
 * SpacialRange.cpp
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#include "PeriodicSpan.h"
#include <algorithm>  // for std::min/max
#include <assert.h>
#include <cmath>

namespace sail {

PeriodicSpan::PeriodicSpan() {
  _mean = 0;
  _maxDif = 0;
}

PeriodicSpan::PeriodicSpan(Arrayd values) {
  int count = values.size();
  double f = 1.0/count;
    double x = 0.0;
    double y = 0.0;
    for (int i = 0; i < count; i++) {
      double angle = values[i];
      x += cos(angle);
      y += sin(angle);
    }
    _mean = atan2(y, x);
    _maxDif = 0.0;
    for (int i = 0; i < count; i++) {
      double angle = values[i];
      _maxDif = std::max(_maxDif, std::acos(std::cos(angle - _mean)));
    }
    assert(_maxDif <= M_PI);
  assert(_maxDif > 0);
}

bool PeriodicSpan::intersects(const PeriodicSpan &other) const {
  // Assuming this object has the greater interval. Does it contain any part of the other?
  if (_maxDif >= other._maxDif) {
    double thresh = cos(_maxDif);
    double offs = other._mean - _mean;
    return (cos(offs) >= thresh) ||
           (cos(offs - other._maxDif) >= thresh) ||
           (cos(offs + other._maxDif) >= thresh);
  } else {
    return other.intersects(*this);
  }
}


} /* namespace sail */
