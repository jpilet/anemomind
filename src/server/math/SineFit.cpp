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
    const Sine::Sample &xy) {
  double a[3] = {
      xy.weight*cos(omega*xy.angle),
      xy.weight*sin(omega*xy.angle),
      xy.weight*1.0};
  double b = xy.weight*xy.y;
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
    const Array<Sine::Sample> &data) {
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
