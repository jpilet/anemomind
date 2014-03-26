/*
 *  Created on: 25 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Statistics.h"
#include <assert.h>
#include <cmath>
#include <iostream>

namespace sail {

Statistics::Statistics() :
  _count(0), _valueSum(0.0), _value2Sum(0.0) {
}

void Statistics::add(double value) {
  _count++;
  _valueSum += value;
  _value2Sum += value*value;
}

bool Statistics::defined() const {
  return _count > 0;
}

double Statistics::mean() const {
  assert(defined());
  return _valueSum/_count;
}

double Statistics::variance() const { // E[x²] - E²[x]
  assert(defined());
  double mu = mean();
  return _value2Sum/_count - mu*mu;
}

double Statistics::stddev() const {
  return sqrt(variance());
}

std::ostream &operator<<(std::ostream &s, const Statistics &st) {
  s << "Statistics(mean=" << st.mean() << " stddev=" << st.stddev() << ")";
  return s;
}


} /* namespace sail */
