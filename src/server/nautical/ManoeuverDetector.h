/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MANOEUVERDETECTOR_H_
#define MANOEUVERDETECTOR_H_

#include <server/common/TimeStamp.h>

namespace sail {

class ManoeuverDetector {
 public:
  ManoeuverDetector();

  class Manoeuver {
   public:
    Manoeuver() {}
    Manoeuver(Duration<double> from, Duration<double> to) :
      _from(from), _to(to) {}
   private:
    Duration<double> _from, _to;
  };

  Array<Manoeuver> detect(Array<Duration<double> > times,
      Array<Angle<double> > angles) const;
 private:
  double _regWeight;
  int _minManoeuverCount;
  double _initManoeuverThresh;
  double _stableThresh;
};

}

#endif /* MANOEUVERDETECTOR_H_ */
