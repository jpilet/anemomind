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

#include <server/common/math.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/ExpLine.h>

namespace sail {

template <typename T>
class SpeedCalib {
 public:
  static constexpr bool withExp = true;

  static Sigmoid<T> kSpan() {
    double marg = 0.1;
    return Sigmoid<T>(1.0 - marg, 1.0 + marg);
  }

  // These functions can be used to map a variable in R
  // to a subset.
  //
  // Caution: For x = 0, the derivative of lowerBound w.r.t. x is 0.
  static T lowerBounded(T x, T lb = 0) {
    return expline(x) + lb;
  }


  SpeedCalib(T kParam, T mParam, T cParam, T alphaParam) :
    _k(kSpan().eval(kParam)), _m(lowerBounded(mParam)), _c(lowerBounded(cParam)) {

    /*
     * Fits alpha in an interval based on the following:
     *
     *    (i)  _alpha > 0 (so that the exponential curve decreases asymptotically towards zero)
     *    (ii) slope of the curve at 0 should not be negative
     */
    _alpha = Sigmoid<T>(0, _k/(_c + 1.0e-9 /*_c > 0, so adding a small number to it eliminates
      division by zero*/)).eval(alphaParam);
  }

  Velocity<T> eval(Velocity<T> vx) {
    T x = vx.metersPerSecond();
    assert(x > 0);
    T y =  scaleCoef()*x + offsetCoef();
    if (withExp) {
      y += nonlinCoef()*exp(-decayCoef()*x);
    }
    return Velocity<T>::metersPerSecond(y);

  }

  T scaleCoef() {return _k;}

  /*
   * Require that f(0) >= 0
   */
  T offsetCoef() {return _m - _c;}

  T nonlinCoef() {return (withExp? _c : 0.0);}

  T decayCoef() {
    return (withExp? _alpha : T(0.0));
  }

  static T initKParam() {return 0;}
  static T initMParam() {return -11;}
  static T initCParam() {return -11;}
  static T initAlphaParam() {return 0.0;}

  // This value can be added to the objective function in order to
  // push the nonlinCoef to zero if decayCoef is close to 0.
  T ambiguityPenalty() {
    return (1.0e-8)*(nonlinCoef()/(1.0e-12 + decayCoef()));
  }
 private:
  T _k, _m, _c, _alpha;
};

} /* namespace sail */

#endif /* SPEEDCALIB_H_ */
