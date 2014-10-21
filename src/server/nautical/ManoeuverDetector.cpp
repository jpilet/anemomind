/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ManoeuverDetector.h"

namespace sail {

ManoeuverDetector::ManoeuverDetector() :
  _regWeight(1000), _minManoeuverCount(8),
  _initManoeuverThresh(1.0e6), _stableThresh(0.1) {}

Array<Manoeuver> detect(Array<Duration<double> > times,
    Array<Angle<double> > angles) const;


}
