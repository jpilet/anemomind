/*
 * AxisTicks.cpp
 *
 *  Created on: 22 Oct 2016
 *      Author: jonas
 */

#include <server/plot/AxisTicks.h>
#include <server/common/String.h>

namespace sail {

BasicTickIterator::BasicTickIterator(int e, const std::string &unit) :
    _exponent(e), _unit(unit) {}

BasicTickIterator BasicTickIterator::coarser() const {
  return BasicTickIterator(_exponent + 1, _unit);
}

BasicTickIterator BasicTickIterator::finer() const {
  return BasicTickIterator(_exponent - 1, _unit);
}

double BasicTickIterator::computeFracIndex(double value) const {
  return value/tickSpacing();
}

AxisTick<double> BasicTickIterator::get(int index) const {
  auto x = index*tickSpacing();
  auto label = stringFormat("%.3g %s", x, _unit.c_str());
  return AxisTick<double>{x, label};
}

double BasicTickIterator::tickSpacing() const {
  return std::pow(10.0, double(_exponent));
}













}
