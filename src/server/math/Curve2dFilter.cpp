/*
 * Curve2dFilter.cpp
 *
 *  Created on: 30 Mar 2017
 *      Author: jonas
 */

#include "Curve2dFilter.h"
#include <server/math/spline/SplineUtils.h>
#include <server/math/band/BandedIrls.h>
#include <server/math/band/BandedIrlsUtils.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/TimedValue.h>
#include <server/common/Functional.h>


namespace sail {
namespace Curve2dFilter {

using namespace BandedIrls;

Eigen::Matrix<double, 1, 2> mat1x2(double x, double y) {
  Eigen::Matrix<double, 1, 2> dst;
  dst << x, y;
  return dst;
}

std::string toString(const Basis::Weights& w) {
  std::stringstream ss;
  for (int i = 0; i < w.dim; i++) {
    ss << "\n  i=" << w.inds[i] << " w="<< w.weights[i];
  }
  return ss.str();
}


template <typename T>
TimedValue<T> getMaxNorm(
    const PhysicalTemporalSplineCurve<T>& c) {
  auto m = c.timeMapper();
  TimedValue<T> maxv(TimeStamp(), 0.0*c.unit());
  auto ispan = c.indexSpan();
  for (auto i: ispan) {
    auto t = m.toTimeStamp(i);

    auto x = sqrt(sqr(c.evaluate(0, t)/c.unit())
        + sqr(c.evaluate(1, t)/c.unit()))*c.unit();

    if (x > maxv.value) {
      maxv = TimedValue<T>(t, x);
    }
  }
  return maxv;
}

TimedValue<Acceleration<double>> Results::getMaxAcceleration() const {
  return getMaxNorm(curve.derivative().derivative());
}

Span<TimeStamp> getReliableSpan(
    const Array<TimeStamp>& inlierPositionTimes) {
  return inlierPositionTimes.empty()?
      Span<TimeStamp>() : Span<TimeStamp>(
          inlierPositionTimes.first(),
          inlierPositionTimes.last());
}

TimedValue<Velocity<double>> Results::getMaxSpeed() const {
  return getMaxNorm(curve.derivative());
}




struct BasisData {
  BasisData(const TimeMapper& mapper) {
    basis = Basis(mapper.sampleCount());
    derivatives = basis.derivatives();
    speed = derivatives[1];
    acceleration = derivatives[2];
    powers = makePowers(derivatives.size(), 1.0/mapper.period().seconds());
  }
  static constexpr int dim = Basis::Weights::dim;

  Basis basis;
  Array<Basis> derivatives;
  Basis speed;
  Basis acceleration;
  Array<double> powers;
};

typedef RobustCost<1, BasisData::dim, 2> DataCostBase;

class DataCost : public DataCostBase {
public:
  TimeStamp time;
  using DataCostBase::DataCostBase;
};




Cost::Ptr toCommonCost(const std::shared_ptr<DataCost>& x) {
  return std::static_pointer_cast<Cost>(x);
}

Array<Cost::Ptr> toCommonCosts(const Array<std::shared_ptr<DataCost>>& src) {
  return map(src, &toCommonCost);
}

Array<std::shared_ptr<DataCost>> makePositionCosts(
    const TimeMapper& mapper,
    const Array<TimedValue<Vec2<Length<double>>>>& positions,
    const Settings& settings,
    const BasisData& b) {
  ArrayBuilder<std::shared_ptr<DataCost>> costs(positions.size());

  ExponentialWeighting weights(settings.iterations,
      settings.initialWeight, settings.finalWeight);

  auto span = b.basis.raw().dataSpan();
  for (auto p: positions) {
    double t = mapper.toRealIndex(p.time);
    if (span.contains(t)) {
      CoefsWithOffset<Basis> coefs(b.basis.build(t));
      auto x = std::make_shared<DataCost>(
                coefs.offset, coefs.coefs,
                mat1x2(p.value[0].meters(), p.value[1].meters()),
                weights, settings.inlierThreshold.meters());
      x->time = p.time;
      costs.add(x);
    }
  }
  return costs.get();
}


Array<std::shared_ptr<DataCost>> makeMotionCosts(
    const TimeMapper& mapper,
    const Array<TimedValue<Vec2<Velocity<double>>>>& motions,
    const Settings& settings,
    const BasisData& b) {
  ArrayBuilder<std::shared_ptr<DataCost>> costs(motions.size());

  ExponentialWeighting weights(settings.iterations,
      settings.initialWeight, settings.finalWeight);
  auto span = b.basis.raw().dataSpan();
  double dweight = b.powers[1];
  double period = mapper.period().seconds();
  for (auto m: motions) {
    double t = mapper.toRealIndex(m.time);
    if (span.contains(t)) {
      CoefsWithOffset<Basis> coefs(b.speed.build(t));
      costs.add(std::make_shared<DataCost>(
          coefs.offset, period*dweight*coefs.coefs,
          period*mat1x2(
              m.value[0].metersPerSecond(),
              m.value[1].metersPerSecond()),
          weights,
          settings.inlierThreshold.meters())); // Really weigh it here?
    }
  }
  return costs.get();
}

Array<Cost::Ptr> makeRegCosts(
    double weight,
    const TimeMapper& mapper,
    const Settings& settings,
    const BasisData& b) {
  ArrayBuilder<Cost::Ptr> costs(mapper.sampleCount());
  auto regWeight = weight*b.powers[2];
  for (int i = 0; i < mapper.sampleCount(); i++) {
    CoefsWithOffset<Basis> coefs(b.acceleration.build(i));
    costs.add(Cost::Ptr(new StaticCost<1, BasisData::dim, 2>(
        coefs.offset, regWeight*coefs.coefs, mat1x2(0, 0))));
  }
  return costs.get();
}

Array<TimeStamp> getInlierPositionTimes(
    const Array<std::shared_ptr<DataCost>>& positionCosts,
    const MDArray2d& X) {
  ArrayBuilder<TimeStamp> dst(positionCosts.size());
  for (const auto& c: positionCosts) {
    if (c->isInlier(X)) {
      dst.add(c->time);
    }
  }
  return dst.get();
}

int toClamped(const TimeMapper& m, TimeStamp t) {
  return clamp(m.toIntegerIndex(t), 0, m.sampleCount());
}

Span<int> mapToIndexSpan(const TimeMapper& m,
    const Span<TimeStamp>& src) {
  return src.initialized()?
      Span<int>(toClamped(m, src.minv()), 1+toClamped(m, src.maxv()))
      : Span<int>();
}

Results optimize(
  const TimeMapper& mapper,
  const Array<TimedValue<Vec2<Length<double>>>>& positions,
  const Array<TimedValue<Vec2<Velocity<double>>>>& motions,
  const Settings& settings,
  const MDArray2d& Xinit) {
  if (positions.empty()) {
    LOG(ERROR) << "No input positions";
    return Results();
  }

  BasisData b(mapper);

  auto positionCosts = makePositionCosts(
      mapper, positions, settings, b);
  auto motionCosts = makeMotionCosts(
      mapper, motions, settings, b);

  auto dataCosts = concat(Array<Array<Cost::Ptr>>({
    toCommonCosts(positionCosts),
    toCommonCosts(motionCosts)
  }));

  BandedIrls::Results solution;
  Array<TimeStamp> inlierTimes;
  for (auto w: settings.regWeights) {
    auto costs = concat(Array<Array<Cost::Ptr>>{
      dataCosts, makeRegCosts(w, mapper, settings, b)
    });
    BandedIrls::Settings birls;
    solution = solve(birls, costs, solution.X);

    if (!solution.OK()) {
      LOG(ERROR) << "Curve filter failed";
      return Results();
    }
    inlierTimes = getInlierPositionTimes(positionCosts, solution.X);
    if (inlierTimes.size() < settings.minimumInlierCount) {
      LOG(ERROR) << "Not enough inlier positions";
      return Results();
    }
  }
  auto reliableSpan = getReliableSpan(inlierTimes);
  auto reliableIndices = mapToIndexSpan(mapper, reliableSpan);
  if (!reliableIndices.initialized() || reliableIndices.width() == 0) {
    LOG(ERROR) << "No reliable data";
    return Results();
  }

  return Results{
    solution,
    positionCosts.size(),
    inlierTimes,
    Results::Curve(
        mapper,
        SplineCurve(
            b.basis,
            solution.X.sliceRowsTo(mapper.sampleCount())), 1.0_m,
            reliableIndices)
  };
}



}
} /* namespace sail */
