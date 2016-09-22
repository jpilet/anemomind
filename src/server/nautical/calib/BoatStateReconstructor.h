/*
 * StateReconstructor.h
 *
 *  Created on: 22 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_
#define SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_

#include <server/nautical/calib/Fitness.h>
#include <server/math/BandedLevMar.h>

namespace sail {

template <typename T, bool RecoverGpsMotion>
class BoatStateReconstructor {
public:
  BoatStateReconstructor(
      TimeStamp offsetTime,
      Duration<double> samplingPeriod,
      int sampleCount) : _offsetTime(offsetTime),
        _samplingPeriod(samplingPeriod),
        _sampleCount(sampleCount) {}

  template <DataCode code>
  void addObservation(
      TimedValue<typename TypeForCode<code>::type> &x) {
    double realIndex = (x.time - _offsetTime)/_samplingPeriod;
    _problem.addCost(new BoatStateFitness<code>(realIndex, x.value));
  }
private:
  TimeStamp _offsetTime;
  Duration<double> _samplingPeriod;
  int _sampleCount;
  BandedLevMar::Problem<T> _problem;
};


}



#endif /* SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_ */
