/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarCurves.h>
#include <server/common/LineKM.h>
#include <server/common/MDArray.h>
#include <server/math/PolarCoordinates.h>
#include <server/nautical/polar/PolarDensity.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <server/plot/extra.h>

namespace sail {

Velocity<double> PolarCurves::Vertex::x() const {
  return calcPolarX(true, _boatSpeed, _twa);
}

Velocity<double> PolarCurves::Vertex::y() const {
  return calcPolarY(true, _boatSpeed, _twa);
}

PolarCurves PolarCurves::fromDensity(const PolarDensity &density, Velocity<double> tws,
    int twaCount, Velocity<double> maxBoatSpeed, int bsCount, double quantile) {
  LineKM twa(0, twaCount, 0.0, 360.0);
  Array<Vertex> pts(twaCount+1);
  for (int i = 0; i < twaCount; i++) {
    Angle<double> alpha = Angle<double>::degrees(twa(i));
    LOG(INFO) << stringFormat("Look up vertex %d of %d", i+1, twaCount);
    pts[i] = Vertex(alpha,
               density.lookUpBoatSpeed(tws, alpha,
               maxBoatSpeed, bsCount, quantile));
  }
  LOG(INFO) << "Done computing curve";
  pts.last() = pts.first();
  return PolarCurves(tws, Array<Array<Vertex> >::args(pts));
}

namespace {
  MDArray2d getCurveData(Array<PolarCurves::Vertex> curve) {
    int count = curve.size();
    MDArray2d data(count, 2);
    for (int i = 0; i < count; i++) {
      PolarCurves::Vertex &v = curve[i];
      data(i, 0) = v.x().knots();
      data(i, 1) = v.y().knots();
    }
    return data;
  }

  void plotCurve(Array<PolarCurves::Vertex> curve, GnuplotExtra *dst) {
    MDArray2d data = getCurveData(curve);
    dst->plot(data);
  }
}

void PolarCurves::plot(GnuplotExtra *dst) {
  for (auto curve: _curves) {
    plotCurve(curve, dst);
  }
}


} /* namespace mmm */
