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

template <typename T>
struct BandWidthForType {};

template <>
struct BandWidthForType<Angle<double> > {
  static Angle<double> get() {
    return Angle<double>::degrees(5.0);
  }
};

template <>
struct BandWidthForType<Velocity<double> > {
  static Velocity<double> get() {
    return Velocity<double>::knots(0.5);
  }
};

template <DataCode code>
struct BandWidth :
    BandWidthForType<typename TypeForCode<code>::type>{};

template <DataCode code, bool RecoverGpsMotion>
class BoatStateFitness {
public:
  static const int windParamCount = 2;
  static const int currentParamCount = 2;
  static const int orientationParamCount = 3;
  static const int
  static const int inputCount =
private:
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
