/*
 * LineKM.cpp
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#include "LineKM.h"
#include <iostream>
#include <cmath>

namespace sail {

void calcLineKM(double x0, double x1, double y0, double y1, double &k, double &m) {
  k = (y1 - y0)/(x1 - x0);
  m = y0 - k*x0;
}

LineKM::LineKM(double x0, double x1, double y0, double y1) {
  calcLineKM(x0, x1, y0, y1, _k, _m);
}

LineKM::LineKM() {
  _k = 1.0;
  _m = 0.0;
}

LineKM LineKM::identity() {
  return LineKM(1.0, 0.0);
}

LineKM LineKM::constant(double c) {
  return LineKM(0.0, c);
}

double LineKM::operator() (double x) const {
  return _k*x + _m;
}

double LineKM::inv(double x) const {
  return (x - _m)/_k;
}

LineKM LineKM::makeInvFun() const {
  return LineKM(1.0/_k, -_m/_k);
}

void LineKM::makeInterpolationWeights(double x,
    int *indsOut2,
    double *weightsOut2) const {
    double realIndex = inv(x);
    int index = int(floor(realIndex));
    double lambda = realIndex - index;
    indsOut2[0] = index;
    indsOut2[1] = index+1;
    weightsOut2[0] = 1.0 - lambda;
    weightsOut2[1] = lambda;
}


double LineKM::getK() const {
  return _k;
}

double LineKM::getM() const {
  return _m;
}


bool LineKM::operator==(const LineKM &other) const {
  return _k == other._k && _m == other._m;
}

std::ostream &operator<<(std::ostream &s, LineKM line) {
  s << "Line y = " << line.getK() << "*x + " << line.getM();
  return s;
}

} /* namespace sail */
