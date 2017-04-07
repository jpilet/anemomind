/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/MeanAndVar.h>
#include <server/common/math.h>
#include <sstream>
#include <server/common/logging.h>

namespace sail {

void MeanAndVar::add(double value) {
  if (_count == 0) {
    _min = _max = value;
  } else {
    _min = std::min(_min, value);
    _max = std::max(_max, value);
  }

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
  double mini, maxi;
  if (_count == 0) {
    mini = other._min;
    maxi = other._max;
  } else if (other._count == 0) {
    mini = _min;
    maxi = _max;
  } else {
    mini = std::min(_min, other._min);
    maxi = std::max(_max, other._max);
  }

  return MeanAndVar(_count + other._count, _sum + other._sum,
      _sum2 + other._sum2, mini, maxi);
}

std::string MeanAndVar::toString() const {
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

MeanAndVar MeanAndVar::normalize() const {
  CHECK_LT(0, _count);
  return MeanAndVar(1, _sum/_count, _sum2/_count, _min, _max);
}

std::ostream &operator<<(std::ostream &s, const MeanAndVar &x) {
  s << "(mean = " << x.mean() << ", stddev = " << x.standardDeviation()
    << " [" << x.min() << "," << x.max() << "]"
    << ")";
  return s;
}


}
