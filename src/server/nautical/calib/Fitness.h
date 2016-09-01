/*
 * Fitness.h
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 *
 * This code is about generating the residuals for fitting to data.
 */

#ifndef SERVER_NAUTICAL_CALIB_FITNESS_H_
#define SERVER_NAUTICAL_CALIB_FITNESS_H_

#include <device/anemobox/Dispatcher.h>
#include <server/nautical/calib/SensorSet.h>
#include <server/nautical/BoatState.h>

namespace sail {

template <typename T, DataCode code,
  typename Q=typename TypeForCode<code>::type>
void computeResidual(
    const SensorModel<T, code> &sensorModel,
    const BoatState<T> &estimatedState,
    const Q &observedValue) {

}

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
