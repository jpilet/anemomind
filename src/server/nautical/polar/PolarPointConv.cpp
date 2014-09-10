/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarPointConv.h>
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>


namespace sail {

HorizontalMotion<double> calcTW(const Nav &x) {
  double params[TrueWindEstimator::NUM_PARAMS];
  TrueWindEstimator::initializeParameters(params);
  return TrueWindEstimator::computeTrueWind(params, x);
}

PolarPoint makePolarPoint(const Nav &x, int index = -1) {
  Angle<double> dir = x.gpsBearing();
  HorizontalMotion<double> tw = calcTW(x);
  return PolarPoint(calcTws(tw),
      calcTwa(tw, dir), x.gpsSpeed(), index);

}

Array<PolarPoint> navsToPolarPoints(Array<Nav> navs) {
  //return navs.map<PolarPoint>([&](const Nav &x) {return makePolarPoint(x);});
  int count = navs.size();
  Array<PolarPoint> dst(count);
  for (int i = 0; i < count; i++) {
    dst[i] = makePolarPoint(navs[i], i);
  }
  return dst;
}

}
