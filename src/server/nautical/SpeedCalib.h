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
  static constexpr double minK = 0.5;
  static constexpr bool withExp = true;

  // These functions can be used to map a variable in R
  // to a subset.
  static T lowerBound(T x, T lb = 0) {
    return x*x + lb;
  }

  static T lowerUpperBound(T x, T lb, T ub) {
    return lb + (ub - lb)/(1.0 + exp(-x));
  }



  SpeedCalib(T k, T m, T c, T alpha) :
    _k2(lowerBound(k, minK)), _m2(lowerBound(m)), _c2(lowerBound(c)), _r2(lowerBound(alpha)) {}

  T eval(T x) {
    assert(x > 0);
    T y =  scaleCoef()*x + offsetCoef();
    if (withExp) {
      y += nonlinCoef()*exp(-decayCoef()*x);
    }
    return y;

  }
  T evalDeriv(T x) {
    assert(x > 0);
    T y = scaleCoef();
    if (withExp) {
      y += -nonlinCoef()*decayCoef()*exp(-decayCoef()*x);
    }
    return y;
  }

  T scaleCoef() {return _k2;}

  /*
   * Require that f(0) >= 0
   */
  T offsetCoef() {return _m2 - _c2;}

  T nonlinCoef() {return (withExp? _c2 : 0.0);}

  // This complicated way of computing the decayCoef ensures
  // that f'(0) >= 0:
  /*
   *    f'(0) = scaleCoef() - decayCoef()*nonlinCoef() >= 0
   *
   * Set f'(0) = _r2 >= 0. This yields
   *
   *   _r2 = scaleCoef() - decayCoef()*nonlinCoef() <=> decayCoef() = (scaleCoef() - _r2)/nonlinCoef()
   *
   */
  T decayCoef() {
    static_assert(minK > 0, "minK should be greater than 0 in order to avoid division by zero.");
    return (withExp? (scaleCoef() - _r2)/(nonlinCoef() + 1.0e-12) : T(0.0));
  }

  static T initK() {return sqrt(1.0 - minK);}
  static T initM() {return 0.001;}
  static T initC() {return 0.001;}
  static T initAlpha() {return 1.0;}

  // This value can be added to the objective function in order to
  // push the nonlinCoef to zero if decayCoef is close to 0.
  T ambiguityPenalty() {
    return (1.0e-8)*(nonlinCoef()/(1.0e-12 + decayCoef()));
  }
 private:
  T _k2, _m2, _c2, _r2;
};

} /* namespace sail */

#endif /* SPEEDCALIB_H_ */
