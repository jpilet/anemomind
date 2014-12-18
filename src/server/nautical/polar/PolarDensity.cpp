/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarDensity.h>
#include <server/math/PolarCoordinates.h>
#include <server/common/LineKM.h>
#include <server/common/logging.h>

namespace sail {

namespace {
KernelDensityEstimator<3>::Vec toDensityVec(const PolarPoint &p) {
  KernelDensityEstimator<3>::Vec v = KernelDensityEstimator<3>::Vec{ // Map to a cartesian room
    calcPolarX(true, p.boatSpeed(), p.twa()).knots(),
    calcPolarY(true, p.boatSpeed(), p.twa()).knots(),
    p.tws().knots()};
  return v;
  }

Array<KernelDensityEstimator<3>::Vec> makePts(Array<PolarPoint> points) {
  return points.map<KernelDensityEstimator<3>::Vec>(toDensityVec);
}


Array<KernelDensityEstimator<3>::Vec> filter(Array<KernelDensityEstimator<3>::Vec> pts) {
  return pts.slice([&] (const KernelDensityEstimator<3>::Vec &x) {
          for (int i = 0; i < 3; i++) {
            if (std::isnan(x[i])) {
              return false;
            }
          }
          return true;
        });
}

}

PolarDensity::PolarDensity(Velocity<double> bandWidth,
    Array<PolarPoint> points, bool mirrored) :
    _mirrored(mirrored),
    _densityEstimator(bandWidth.knots(),
    filter(makePts(points))) {
    assert(!_densityEstimator.empty());
}

double PolarDensity::densitySub(const PolarPoint &point) const {
  auto v = toDensityVec(point);
  double d = _densityEstimator.density(v);
  if (_mirrored) {
    v[0] = -v[0];
    return d + _densityEstimator.density(v);
  }
  return d;
}

double PolarDensity::density(const PolarPoint &point) const {
  double d = densitySub(point);
  assert(!std::isnan(d));
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

  double at = dsum*quantile;

  double from = densitySamples[0];
  for (int i = 1; i < sampleCount; i++) {
    double to = from + densitySamples[i];
    if (at <= to) {
      LineKM interpolator(from, to, bsKnots(i-1), bsKnots(i));
      return Velocity<double>::knots(interpolator(at));
    }

    from = to;
  }
  return maxBoatSpeed;
}

CumulativeFunction PolarDensity::makeRadialKnotFunction(Vectorize<Velocity<double>, 3> vec,
                    Velocity<double> maxBoatSpeed, int sampleCount) const {
  LineKM bsKnots(0, sampleCount-1, 0.0, maxBoatSpeed.knots());
  Arrayd densitySamples(sampleCount);
  double dsum = 0;
  Angle<double> twa = Angle<double>::radians(atan2(vec[0].knots(), vec[1].knots()));
  for (int i = 0; i < sampleCount; i++) {
    double d = density(PolarPoint(vec[2], twa, Velocity<double>::knots(bsKnots(i))));
    densitySamples[i] = d;
    dsum += d;
  }
  return CumulativeFunction(bsKnots, densitySamples);
}


}
