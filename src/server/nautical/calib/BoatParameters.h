/*
 * BoatParameters.h
 *
 *  Created on: 29 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_BOATPARAMETERS_H_
#define SERVER_NAUTICAL_CALIB_BOATPARAMETERS_H_

namespace sail {

template <typename T>
struct BoatParameters {
  AnglePerVelocity<T> heelConstant;

  // How the leeway angle of the boat
  // depends on heel.
  LeewayConstant<T> leewayConstant;

  SensorDistortionSet<T> sensors;
};

}



#endif /* SERVER_NAUTICAL_CALIB_BOATPARAMETERS_H_ */
