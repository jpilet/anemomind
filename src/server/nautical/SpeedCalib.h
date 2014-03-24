/*
 *  Created on: 2014-03-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Calibration model for speed:
 *    y: True output speed
 *    x: Measured input speed, expected to be positive
 *
 *  y = k*x + m² + c²(1 - e^(-alpha²*x))
 *  dydx = k + alpha²c²e^(-alpha²*x)
 *
 *  x is required to be positive. The reason for this is that
 *  with a realistic speed sensor and for a very low true speed,
 *  a speed of 0 may be registered. This means that several true speeds
 *  map to a measured speed of 0, and thus the function is not invertible at those
 *  those values.
 *
 *  The coefficients are squared to force a particular sign.
 *
 *  Explanation of the calibration constants:
 *   k : Scaling error coefficient. A value of 1 means no scaling error
 *   m : Offset error coefficient. A value of 0 means no offset error
 *   c : Amount of nonlinear effect
 *   alpha : Decay rate of the nonlinear effect
 */

#ifndef SPEEDCALIB_H_
#define SPEEDCALIB_H_

namespace sail {

template <typename T>
class SpeedCalib {
 public:
  SpeedCalib(T k, T m, T c, T alpha) :
    _k(k), _m2(m*m), _c2(c*c), _alpha2(alpha*alpha) {}

  T eval(T x) {
    assert(x > 0);
    return _k*x + _m2 + _c2*(1.0 - exp(-_alpha2*x));
  }
  T evalDeriv(T x) {
    assert(x > 0);
    return _k + _c2*_alpha2*exp(-_alpha2*x);
  }
 private:
  T _k, _m2, _c2, _alpha2;
};

} /* namespace sail */

#endif /* SPEEDCALIB_H_ */
