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
#include <server/common/LineKM.h>

namespace sail {

typedef QuadForm<3, 1, double> QF;

Angle<double> SpacedAngles::at(int n) const{
  return offset + double(n)*step;
}

Angle<double> SpacedAngles::smallest() const {
  double f = -double(offset/step);
  auto a = offset + floor(f)*step;
  auto b = offset + ceil(f)*step;
  return fabs(a) < fabs(b)? a : b;
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

Array<Sine::Sample> Sine::sample(int n,
    Angle<double> from, Angle<double> to) const {
  LineKM m(0, n-1, from.degrees(), to.degrees());
  Array<Sine::Sample> dst(n);
  for (int i = 0; i < n; i++) {
    auto angle = m(i)*1.0_deg;
    dst[i] = Sine::Sample(angle, (*this)(angle));
  }
  return dst;
}



Optional<Sine> fit(double omega,
    const Array<Sine::Sample> &data) {
  QF sum = QF::makeReg(1.0e-9);
  for (auto x: data) {
    sum += makeQuad(omega, x);
  }
  auto p = sum.minimizeEigen();
  for (int i = 0; i < 3; i++) {
    if (!std::isfinite(double(p(i)))) {
      return Optional<Sine>();
    }
  }
  double a = p(0, 0);
  double b = p(1, 0);
  auto C = sqrt(std::max(0.0, a*a + b*b));
  auto phiRad = atan2(a, b);
  auto D = p(2, 0);
  return Sine(C, omega, phiRad*1.0_rad, D);
}


} /* namespace sail */
