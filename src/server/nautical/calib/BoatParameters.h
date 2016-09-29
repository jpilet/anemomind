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
  HeelConstant<T> heelConstant;
  LeewayConstant<T> leewayConstant;
  SensorDistortionSet<T> sensors;
};

}



#endif /* SERVER_NAUTICAL_CALIB_BOATPARAMETERS_H_ */
