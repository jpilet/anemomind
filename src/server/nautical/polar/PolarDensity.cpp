/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarDensity.h>
#include <server/math/PolarCoordinates.h>
#include <server/common/LineKM.h>

namespace sail {

namespace {
KernelDensityEstimator<3>::Vec toDensityVec(const PolarPoint &p) {
    return KernelDensityEstimator<3>::Vec{calcPolarX(true, p.boatSpeed(), p.twa()).knots(),
                                          calcPolarY(true, p.boatSpeed(), p.twa()).knots(),
                                          p.tws().knots()};
  }
}

PolarDensity::PolarDensity(Velocity<double> bandWidth,
    Array<PolarPoint> points, bool mirrored) :
    _mirrored(mirrored),
    _densityEstimator(bandWidth.knots(),
    points.map<KernelDensityEstimator<3>::Vec>([&](const PolarPoint &p) {return toDensityVec(p);})){
}

double PolarDensity::density(const PolarPoint &point) const {
  auto v = toDensityVec(point);
  double d = _densityEstimator.density(v);
  if (_mirrored) {
    v[0] = -v[0];
    return d + _densityEstimator.density(v);
  }
  return d;
}

Velocity<double> PolarDensity::lookUpBoatSpeed(Velocity<double> tws, Angle<double> twa,
    Velocity<double> maxBoatSpeed, int sampleCount, double quantile) const {
  assert(0 <= quantile);
  assert(quantile <= 1.0);
  LineKM bsKnots(0, sampleCount-1, 0.0, maxBoatSpeed.knots());
  Arrayd densitySamples(sampleCount);
  double dsum = 0;
  for (int i = 0; i < sampleCount; i++) {
    double d = density(PolarPoint(tws, twa, Velocity<double>::knots(bsKnots(i))));
    densitySamples[i] = d;
    dsum += d;
  }

  double acc = 0;
  for (int i = 0; i < sampleCount; i++) {
    if (quantile <= acc/dsum) {
      return Velocity<double>::knots(bsKnots(i));
    }
  }
  return maxBoatSpeed;
}



}
