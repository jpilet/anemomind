/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/MeanAndVar.h>
#include <server/common/math.h>
#include <server/common/logging.h>

namespace sail {

void MeanAndVar::add(double value) {
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

double MeanAndVar::variance() const {
  return _sum2/_count - sqr(mean());
}

double MeanAndVar::standardDeviation() const {
  return sqrt(variance());
}

MeanAndVar MeanAndVar::operator+ (const MeanAndVar &other) const {
  return MeanAndVar(_count + other._count, _sum + other._sum,
      _sum2 + other._sum2);
}

std::ostream &operator<<(std::ostream &s, const MeanAndVar &x) {
  s << "(mean = " << x.mean() << ", stddev = " << x.standardDeviation() << ")";
  return s;
}


}
