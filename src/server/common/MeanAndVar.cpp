/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/MeanAndVar.h>

#include <server/common/logging.h>
#include <server/common/math.h>
#include <sstream>

namespace sail {

void MeanAndVar::add(double value) {
  _bounds.extend(value);

  _count++;
  _sum += value;
  _sum2 += value*value;
}

MeanAndVar::MeanAndVar(Arrayd arr) : MeanAndVar() {
  for (int i = 0; i < arr.size(); i++) {
    add(arr[i]);
  }
}

double MeanAndVar::mean() const {
  CHECK_LT(0, _count);
  return _sum/_count;
}

double MeanAndVar::biasedVariance() const {
  return _sum2/_count - sqr(mean());
}

double MeanAndVar::variance() const {
  // http://en.wikipedia.org/wiki/Variance#Sample_variance
  // In matlab: help var
  double bv = biasedVariance();
  if (_count <= 1) {
    return bv;
  } else {
    return (double(_count)/(_count - 1))*biasedVariance();
  }
}

double MeanAndVar::rms() const {
  return sqrt(_sum2/_count);
}

double MeanAndVar::standardDeviation() const {
  return sqrt(variance());
}

MeanAndVar MeanAndVar::operator+ (const MeanAndVar &other) const {
  Span<double> bounds(_bounds);
  bounds.extend(other._bounds);

  return MeanAndVar(_count + other._count, _sum + other._sum,
      _sum2 + other._sum2, bounds);
}

std::string MeanAndVar::toString() const {
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

MeanAndVar MeanAndVar::normalize() const {
  CHECK_LT(0, _count);
  return MeanAndVar(1, _sum/_count, _sum2/_count, _bounds);
}

std::ostream &operator<<(std::ostream &s, const MeanAndVar &x) {
  s << "(mean = " << x.mean() << ", stddev = " << x.standardDeviation()
    << " [" << x.min() << "," << x.max() << "]"
    << ")";
  return s;
}


}
