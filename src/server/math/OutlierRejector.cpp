/*
 * OutlierRejector.cpp
 *
 *  Created on: 3 Nov 2016
 *      Author: jonas
 */

#include <server/math/OutlierRejector.h>
#include <cmath>
#include <assert.h>
#include <limits>
#include <iostream>
#include <server/common/math.h>

namespace sail {

OutlierRejector::OutlierRejector() :
    _alpha(NAN),
    _beta(NAN),
    _sigma(NAN) {}

OutlierRejector::OutlierRejector(
    const Settings &settings) :
      _sigma(settings.sigma),
      _alpha(settings.initialWeight),
      _beta(settings.initialWeight) {}

double computeSlack(
    double residual,
    double previousWeight) {
  return residual/(1.0 + sqr(previousWeight));
}

void OutlierRejector::update(
    double weight, double residual) {
  assert(std::isfinite(_sigma));
  auto a = computeSlack(residual, _alpha);
  auto b = computeSlack(_sigma, _beta);
  auto ar = std::abs(a);
  auto br = std::abs(b);
  auto f = (2.0*weight)/(ar + br);
  std::cout << "  a=" << a << "  b=" << b << "  f=" << f << std::endl;
  _alpha = f*br;
  _beta = f*ar;
}

double OutlierRejector::computeSquaredWeight() const {
  assert(std::isfinite(_sigma));
  return 1.0 - 1.0/(1.0 + sqr(_alpha));
}

double OutlierRejector::computeWeight() const {
  return sqrt(std::abs(computeSquaredWeight()));
}

} /* namespace sail */
