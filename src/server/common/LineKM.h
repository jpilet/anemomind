/*
 * LineKM.h
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#ifndef LINEKM_H_
#define LINEKM_H_

#include <iosfwd>

namespace sail {

// function on the form f(x) = _k*x + _m
class LineKM {
 public:
  LineKM(double x0, double x1, double y0, double y1);
  LineKM(double k, double m) : _k(k), _m(m) {}
  LineKM();
  double operator() (double x) const;
  double inv(double x) const;
  double getK() const;
  double getM() const;
  bool operator==(const LineKM &other) const;
  LineKM makeInvFun() const;
 private:
  double _k, _m;
};

std::ostream &operator<<(std::ostream &s, LineKM line);

void calcLineKM(double x0, double x1, double y0, double y1, double &k, double &m);

} /* namespace sail */

#endif /* LINEKM_H_ */
