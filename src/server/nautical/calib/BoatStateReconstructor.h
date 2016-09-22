/*
 * StateReconstructor.h
 *
 *  Created on: 22 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_
#define SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_

#include <server/nautical/calib/Fitness.h>

namespace sail {

template <typename T, bool RecoverGpsMotion>
class BoatStateReconstructor {
public:
  BoatStateReconstructor(
      TimeStamp offsetTime,
      Duration<double> samplingPeriod,
      int sampleCount);

  template <DataCode code>
  void addObservation(
      TimedValue<typename TypeForCode<code>::type> &x) {

  }
private:
  BandedLevMar::Problem<T> _problem;
};


}



#endif /* SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_ */
