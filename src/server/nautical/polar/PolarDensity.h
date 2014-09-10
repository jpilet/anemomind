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
      Array<PolarPoint> points);

  double density(const PolarPoint &point) const;
 private:
  KernelDensityEstimator<3> _densityEstimator;
};

}

#endif /* POLARDENSITY_H_ */
