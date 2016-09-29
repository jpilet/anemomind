/*
 * BoatParameters.h
 *
 *  Created on: 29 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_BOATPARAMETERS_H_
#define SERVER_NAUTICAL_CALIB_BOATPARAMETERS_H_

#include <server/nautical/calib/SensorSet.h>

namespace sail {

template <typename T>
using HeelConstant = decltype(
    std::declval<Angle<T>>()/std::declval<Velocity<T>>());

template <typename T>
using LeewayConstant = decltype(
    std::declval<Velocity<T>>()*std::declval<Velocity<T>>());

template <typename T>
struct BoatParameters {
  static HeelConstant<T> heelConstantUnit() {
    return Angle<T>::radians(MakeConstant<T>::apply(1.0))/
        Angle<T>::metersPerSecond(MakeConstant<T>::apply(1.0));
  }
  static LeewayConstant<T> leewayConstantUnit() {
    return Angle<T>::metersPerSecond(MakeConstant<T>::apply(1.0))
        *Angle<T>::metersPerSecond(MakeConstant<T>::apply(1.0));
  }

  HeelConstant<T> heelConstant;
  LeewayConstant<T> leewayConstant;
  SensorDistortionSet<T> sensors;

  int paramCount() const {
    return 2 + sensors.paramCount();
  }

  void readFrom(const T *src) {
    heelConstant = src[0]*heelConstantUnit();
    leewayConstant = src[1]*leewayConstantUnit();
    sensors.readFrom(src + 2);
  }

  void writeTo(T *dst) const {
    dst[0] = heelConstant/heelConstantUnit();
    dst[1] = leewayConstant/leewayConstantUnit();
    sensors.writeTo(dst + 2);
  }

  template <typename S>
  BoatParameters<S> cast() const {
    return BoatParameters<S>{
      heelConstant.template cast<S>(),
      leewayConstant.template cast<S>(),
      sensors.template cast<S>()
    };
  }
};

}



#endif /* SERVER_NAUTICAL_CALIB_BOATPARAMETERS_H_ */
