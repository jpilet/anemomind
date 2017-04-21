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
#include <server/math/band/BandedIrlsUtils.h>
#include <server/common/DOMUtils.h>

namespace sail {
namespace Curve2dFilter {

typedef SmoothBoundarySplineBasis<double, 3> Basis;

struct BasisData {
  BasisData() {}
  BasisData(const TimeMapper& mapper) {
    basis = Basis(mapper.sampleCount());
    derivatives = basis.derivatives();
    speed = derivatives[1];
    acceleration = derivatives[2];
  }
  static constexpr int dim = Basis::Weights::dim;

  Basis basis;
  Array<Basis> derivatives;
  Basis speed;
  Basis acceleration;
};

template <typename T>
using Vec2 = sail::Vectorize<T, 2>;

typedef BandedIrls::RobustCost<1, BasisData::dim, 2> DataCostBase;

class DataCost : public DataCostBase {
public:
  TimeStamp time;
  using DataCostBase::DataCostBase;
};

struct RegCost {
  BandedIrls::Cost::Ptr cost;

  // If not null, then this is the actual class of cost
  std::shared_ptr<DataCost> maybeDataCost;

  bool isInlier(const MDArray2d& X) const {
    return !maybeDataCost || maybeDataCost->isInlier(X);
  }

  TimeStamp getTime() const {
    return maybeDataCost? maybeDataCost->time : TimeStamp();
  }

  RegCost() {}

  RegCost(const std::shared_ptr<DataCost>& c)
    : cost(std::static_pointer_cast<BandedIrls::Cost>(c)),
      maybeDataCost(c) {}

  RegCost(const BandedIrls::Cost::Ptr& c) : cost(c) {}
};



struct Settings {
  // Parameters related to the optimization algorithm
  DOM::Node output;
  int minimumInlierCount = 30;
  int iterations = 30;
  double initialWeight = 0.1;
  double finalWeight = 10000.0;

  // Parameters related to the objective function
  Length<double> inlierThreshold = 10.0_m;
  Array<double> regWeights = {1.0};
  Acceleration<double> regSigma = -1.0_mps2;


  // Helper methods

  // If this is true, we allow for "outliers" in the regularization
  // That is useful in order to detect discontinuities in the curve
  // where we should cut it again.
  bool robustRegularization() const;
};

struct Results {
  typedef PhysicalTemporalSplineCurve<Length<double>> Curve;

  bool empty() const {return reliableIndices.size() == 0;}
  bool OK() const {return !empty();}

  BandedIrls::Results optimizerOutput;
  int positionsCountUsedForOptimization;
  Array<TimeStamp> inlierPositionTimes;
  TimeMapper timeMapper;
  BasisData basis;
  MDArray2d X;
  Spani reliableIndices;
  Array<RegCost> regCosts;

  Curve curve() const;

  Array<Span<TimeStamp>> segmentSpans(
      Duration<double> margin) const;

  Array<Curve> segmentCurves(Duration<double> margin) const;
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
