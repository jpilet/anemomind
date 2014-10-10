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

  /*
   * Finds two contiguous integers, I[0] and I[1] with I[0] + 1 = I[1],
   * and two weights W[0] and W[1] with W[0]+W[1] = 1.0,
   *
   * so that x = W[0]*(*this)(I[0]) + W[1]*(*this)(I[1])
   *
   * Useful for piecewise linear interpolation
   */
  void makeInterpolationWeights(double x,
      int *indsOut2,
      double *weightsOut2) const;
 private:
  double _k, _m;
};

std::ostream &operator<<(std::ostream &s, LineKM line);

void calcLineKM(double x0, double x1, double y0, double y1, double &k, double &m);

} /* namespace sail */

#endif /* LINEKM_H_ */
