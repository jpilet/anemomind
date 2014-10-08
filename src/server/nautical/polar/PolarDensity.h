/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARDENSITY_H_
#define POLARDENSITY_H_

#include <server/math/DensityEstimator.h>
#include <server/nautical/polar/PolarPoint.h>
#include <server/common/string.h>
#include <iostream>
#include <server/math/CumulativeFunction.h>

namespace sail {

// Wrapper class using PhysicalQuantities to access the density.
class PolarDensity {
 public:
  PolarDensity(Velocity<double> bandWidth,
      Array<PolarPoint> points, bool mirrored);

  double density(const PolarPoint &point) const;

  template <typename T>
  T density(const Velocity<T> *xyz) const {
    T xyzKnots[3];
    for (int i = 0; i < 3; i++) {
      xyzKnots[i] = xyz[i].knots();
    }

    T d = _densityEstimator.density(xyzKnots);
    constexpr double tol = 1.0e-9;
    assert(-tol < d);
    if (_mirrored) {
      xyzKnots[0] = -xyzKnots[0];
      T e = _densityEstimator.density(xyzKnots);
      assert(-tol <= e);
      T f = 0.5*(d + e);
      return f;
    }
    return d;
  }

  template <typename T>
  T lsqResidue(const Velocity<T> *xyz) const {

    // Pull the square root out of it, because we are going to square it
    // again in the optimizer :-)
    T dens = density(xyz);
    T maxDens = _densityEstimator.maxDensity();
    constexpr double tol = 1.0e-9;
    assert(-tol < dens);
    assert(-tol < maxDens);
    assert(dens < maxDens);
    T dif = maxDens - dens;
    assert(-tol < dif);
    if (dif < 0) {
      return 0;
    }
    return sqrt(dif);
  }

  Velocity<double> lookUpBoatSpeed(Velocity<double> tws, Angle<double> twa,
      Velocity<double> maxBoatSpeed, int sampleCount, double quantile) const;

  CumulativeFunction makeRadialKnotFunction(Vectorize<Velocity<double>, 3> vec,
                    Velocity<double> maxBoatSpeed, int sampleCount) const;
 private:
  double densitySub(const PolarPoint &point) const;
  bool _mirrored;
  KernelDensityEstimator<3> _densityEstimator;
};

}

#endif /* POLARDENSITY_H_ */
