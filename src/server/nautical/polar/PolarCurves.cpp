/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarCurves.h>
#include <server/common/LineKM.h>
#include <server/nautical/polar/PolarDensity.h>

namespace sail {

PolarCurves PolarCurves::fromDensity(const PolarDensity &density, Velocity<double> tws,
    int twaCount, Velocity<double> maxBoatSpeed, int bsCount, double quantile) {
  LineKM twa(0, twaCount, 0.0, 360.0);
  Array<Vertex> pts(twaCount+1);
  for (int i = 0; i < twaCount; i++) {
    Angle<double> alpha = Angle<double>::degrees(twa(i));
    pts[i] = Vertex(alpha,
               density.lookUpBoatSpeed(tws, alpha,
               maxBoatSpeed, bsCount, quantile));
  }
  pts.last() = pts.first();
  return PolarCurves(tws, Array<Array<Vertex> >::args(pts));
}


} /* namespace mmm */
