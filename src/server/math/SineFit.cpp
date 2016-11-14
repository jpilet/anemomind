/*
 * SineFit.cpp
 *
 *  Created on: 14 Nov 2016
 *      Author: jonas
 */

#include <server/math/SineFit.h>
#include <server/math/QuadForm.h>
#include <iostream>
#include <server/common/ArrayIO.h>

namespace sail {

typedef QuadForm<3, 1, double> QF;

Angle<double> SpacedAngles::at(int n) const{
  return offset + double(n)*step;
}

QF makeQuad(double omega,
    const std::pair<Angle<double>, double> &xy) {
  double a[3] = {
      cos(omega*xy.first),
      sin(omega*xy.first),
      1.0};
  double b = xy.second;
  return QF::fit(a, &b);
}

SpacedAngles findExtrema(const Sine &x) {
  double f = 1.0/x.omega();
  return SpacedAngles{
    f*(90.0_deg - x.phi()),
    f*180.0_deg
  };
}

SpacedAngles minimize(const Sine &x) {
  auto e = findExtrema(x);
  auto offset = e.offset +
      (x(e.at(0)) < x(e.at(1))? 0.0_deg : e.step);
  return SpacedAngles{offset, 2.0*e.step};
}



Optional<Sine> fit(double omega,
    const Array<std::pair<Angle<double>, double>> &data) {
  QF sum = QF::makeReg(1.0e-9);
  for (auto x: data) {
    sum += makeQuad(omega, x);
  }
  auto p = sum.minimizeEigen();
  double a = p(0, 0);
  double b = p(1, 0);
  auto C = sqrt(std::max(0.0, a*a + b*b));
  auto phiRad = atan2(a, b);
  auto D = p(2, 0);
  return Sine(C, omega, phiRad*1.0_rad, D);
}


} /* namespace sail */
