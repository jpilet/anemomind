/*
 * SineFit.cpp
 *
 *  Created on: 14 Nov 2016
 *      Author: jonas
 */

#include <server/math/SineFit.h>
#include <server/math/QuadForm.h>

namespace sail {

typedef QuadForm<3, 1, double> QF;

Angle<double> SpacedAngles::evaluate(int n) const{
  return offset + double(n)*step;
}

SpacedAngles minimize(const Sine &x) {
  return SpacedAngles();
}

QF makeQuad(double omega,
    const std::pair<Angle<double>, double> &xy) {
  double a[3] = {cos(omega*xy.first),
      sin(omega*xy.first), 1.0};
  double b = xy.second;
  return QF::fit(a, &b);
}

Optional<Sine> fit(double omega,
    const Array<std::pair<Angle<double>, double>> &data) {
  QF sum = QF::makeReg(1.0e-9);
  for (auto x: data) {
    sum += makeQuad(omega, x);
  }
  auto p = sum.minimize();
  if (p.empty()) {
    return Optional<Sine>();
  }
  auto a = p(0, 0);
  auto b = p(1, 0);
  auto C = sqrt(std::max(0.0, a*a + b*b));
  auto phiRad = atan2(a, b);
  auto D = p(2, 0);
  return Sine(C, omega, phiRad*1.0_rad, D);
}


} /* namespace sail */
