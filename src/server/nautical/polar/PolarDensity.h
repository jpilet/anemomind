/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARDENSITY_H_
#define POLARDENSITY_H_

#include <server/math/DensityEstimator.h>
#include <server/nautical/polar/PolarPoint.h>

namespace sail {

// Wrapper class using PhysicalQuantities to access the density.
class PolarDensity {
 public:
  PolarDensity(Velocity<double> bandWidth,
      Array<PolarPoint> points, bool mirrored);

  double density(const PolarPoint &point) const;

  Velocity<double> lookUpBoatSpeed(Velocity<double> tws, Angle<double> twa,
      Velocity<double> maxBoatSpeed, int sampleCount, double quantile) const;
 private:
  double densitySub(const PolarPoint &point) const;
  bool _mirrored;
  KernelDensityEstimator<3> _densityEstimator;
};

}

#endif /* POLARDENSITY_H_ */
