/*
 * DriftModel.h
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_DRIFTMODEL_H_
#define SERVER_MATH_DRIFTMODEL_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

enum class DriftModelType {
  AngleAddedToHeading
};

template <typename T>
T driftPolynomial(T x, T alpha) {
  return x*sqr(x - alpha)*sqr(x + alpha);
}

struct BasicDriftModel {
  static const DriftModelType type = DriftModelType::AngleAddedToHeading;

  static const int paramCount = 1;

  template <typename T>
  static Angle<T> eval(const BoatState<T> &bs, const T *parameters) {
    T scale = parameters[0];
    Angle<T> twa = bs.twa();
    auto twaRad = twa.normalizedAt0().radians();
    T alpha = T(0.5*M_PI);
    if (fabs(twaRad) <= alpha) {
      return Angle<T>::radians(scale*driftPolynomial(twaRad, alpha));
    }
    return Angle<T>::radians(T(0.0));
  }
};

}



#endif /* SERVER_MATH_DRIFTMODEL_H_ */
