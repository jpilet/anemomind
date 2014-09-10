/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarDensity.h>
#include <server/math/PolarCoordinates.h>

namespace sail {

namespace {
KernelDensityEstimator<3>::Vec toDensityVec(const PolarPoint &p) {
    return KernelDensityEstimator<3>::Vec{calcPolarX(true, p.boatSpeed(), p.twa()).knots(),
                                          calcPolarY(true, p.boatSpeed(), p.twa()).knots(),
                                          p.tws().knots()};
  }
}

PolarDensity::PolarDensity(Velocity<double> bandWidth,
    Array<PolarPoint> points) :
    _densityEstimator(bandWidth.knots(),
    points.map<KernelDensityEstimator<3>::Vec>([&](const PolarPoint &p) {return toDensityVec(p);})){
}

double PolarDensity::density(const PolarPoint &point) const {
  return _densityEstimator.density(toDensityVec(point));
}


}
