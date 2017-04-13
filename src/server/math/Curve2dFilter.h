/*
 * Curve2dFilter.h
 *
 *  Created on: 30 Mar 2017
 *      Author: jonas
 */

#ifndef SERVER_MATH_CURVE2DFILTER_H_
#define SERVER_MATH_CURVE2DFILTER_H_

#include <server/math/spline/SplineUtils.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <utility>
#include <server/math/band/BandedIrls.h>

namespace sail {
namespace Curve2dFilter {

typedef SmoothBoundarySplineBasis<double, 3> Basis;

template <typename T>
using Vec2 = sail::Vectorize<T, 2>;

struct Settings {
  int minimumInlierCount = 30;
  int iterations = 30;
  double initialWeight = 0.1;
  double finalWeight = 10000.0;
  Length<double> inlierThreshold = 10.0_m;
  Array<double> regWeights = {1.0};
};

struct Results {
  typedef PhysicalTemporalSplineCurve<Length<double>> Curve;

  bool OK() const {return !curve.empty();}
  bool empty() const {return curve.empty();}

  TimedValue<Velocity<double>> getMaxSpeed() const;
  TimedValue<Acceleration<double>> getMaxAcceleration() const;

  BandedIrls::Results optimizerOutput;
  int positionsCountUsedForOptimization;
  Array<TimeStamp> inlierPositionTimes;
  Curve curve;
};

Results optimize(
  const TimeMapper& mapper,
  const Array<TimedValue<Vec2<Length<double>>>>& positions,
  const Array<TimedValue<Vec2<Velocity<double>>>>& motions,
  const Settings& options,
  const MDArray2d& Xinit = MDArray2d());

}
} /* namespace sail */

#endif /* SERVER_MATH_CURVE2DFILTER_H_ */
