/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MANOEUVERDETECTOR_H_
#define MANOEUVERDETECTOR_H_

#include <server/common/TimeStamp.h>
#include <server/common/Array.h>

namespace sail {

class ManoeuverDetector {
 public:
  ManoeuverDetector();

  class Manoeuver {
   public:
    Manoeuver() : _strength(NAN) {}
    Manoeuver(double strength_, Duration<double> peakLoc_,
        Duration<double> from_, Duration<double> to_) :
      _strength(strength_), _from(from_), _to(to_), _peakLoc(peakLoc_) {}

    double strength() const {
      return _strength;
    }

    Duration<double> peakLoc() const {
      return _peakLoc;
    }
   private:
    double _strength;
    Duration<double> _from, _to, _peakLoc;
  };

  Array<Manoeuver> detect(Array<Duration<double> > times,
      Array<Angle<double> > angles) const;
 private:
  double _regWeight;
  int _minManoeuverCount;

  // These apply to the derivative w.r.t. time, in degrees per seconds.
  double _initManoeuverThresh;
  double _stableThresh;
};

}

#endif /* MANOEUVERDETECTOR_H_ */
