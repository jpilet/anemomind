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

//#include <server/common/math.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

template <typename T>
class SpeedCalib {
 public:
  static constexpr bool withExp = false;

  SpeedCalib(T kParam, T mParam, T cParam, T alphaParam) :
    _k(kParam + 1.0), _m(mParam), _c(cParam), _alpha(alphaParam) {
  }

  Velocity<T> eval(Velocity<T> vx) {
    T x = vx.metersPerSecond();
    T y =  _k*x + _m;
    if (withExp) {
      y += _c*exp(_alpha*x);
    }
    return Velocity<T>::metersPerSecond(y);

  }

  static T initKParam() {return T(0);}
  static T initMParam() {return T(0);}
  static T initCParam() {return T(0);}
  static T initAlphaParam() {return T(0);}

 //private:
  T _k, _m, _c, _alpha;
};

} /* namespace sail */

#endif /* SPEEDCALIB_H_ */
