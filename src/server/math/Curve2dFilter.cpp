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
#include <server/common/string.h>
#include <server/math/SampleUtils.h>
#include <server/nautical/BoatSpecificHacks.h>
#include <server/transducers/Transducer.h>


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


Span<TimeStamp> getReliableSpan(
    const Array<TimedPosition>& inlierPositionTimes) {
  return inlierPositionTimes.empty()?
      Span<TimeStamp>() : Span<TimeStamp>(
          inlierPositionTimes.first().time,
          inlierPositionTimes.last().time);
}


/*
 * About speed and acceleration.
 *
 * Assuming that a basis codes a physical quantity
 * as
 *
 *   Y = unit*curve.evaluate(mapper.toRealIndex(x))
 *
 * Then the derivative is
 *
 *   dYdT = unit*curve.derivative()
 *     .evaluate(mapper.toRealIndex(x))/mapper.period()
 *
 * But maybe we want to convert this quantity back to the previous one,
 * so we multiply it by mapper.period() again
 *
 * mapper.period()*dYdT = unit*curve
 *     .derivative().evaluate(mapper.toRealIndex(x))
 *
 * That is why we don't apply any weighting in particular for velocity
 * fitness and acceleration.
 *
 */



Cost::Ptr toCommonCost(const std::shared_ptr<DataCost>& x) {
  return std::static_pointer_cast<Cost>(x);
}

Array<Cost::Ptr> toCommonCosts(const Array<std::shared_ptr<DataCost>>& src) {
  return transduce(src, trMap(&toCommonCost), IntoArray<Cost::Ptr>());
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
      x->position = p.value;
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
  double period = mapper.period().seconds();
  for (auto m: motions) {
    double t = mapper.toRealIndex(m.time);
    if (span.contains(t)) {
      CoefsWithOffset<Basis> coefs(b.speed.build(t));
      costs.add(std::make_shared<DataCost>(
          coefs.offset, coefs.coefs, // Don't multipy by period here...
          period*mat1x2( // ...but multiply by period here.
              m.value[0].metersPerSecond(),
              m.value[1].metersPerSecond()),
          weights,
          settings.inlierThreshold.meters(),
          settings.motionWeight));
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
  for (int i = 0; i < mapper.sampleCount(); i++) {
    CoefsWithOffset<Basis> coefs(b.acceleration.build(i));
    costs.add(Cost::Ptr(new StaticCost<1, BasisData::dim, 2>(
        coefs.offset, weight*coefs.coefs, mat1x2(0, 0))));
  }
  return costs.get();
}



Array<TimedPosition> getInlierPositions(
    const Array<std::shared_ptr<DataCost>>& positionCosts,
    const MDArray2d& X) {
  ArrayBuilder<TimedPosition> dst(positionCosts.size());
  for (const auto& c: positionCosts) {
    if (c->isInlier(X)) {
      dst.add(TimedPosition(c->time, c->position));
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

  auto dataCosts = transduce(
      Array<Array<Cost::Ptr>>({
        toCommonCosts(positionCosts),
        toCommonCosts(motionCosts)
      }), trCat(), IntoArray<Cost::Ptr>());

  BandedIrls::Results solution;
  Array<TimedValue<Vec2<Length<double>>>> inlierPositions;
  BandedIrls::Settings birls;
  for (auto w: settings.regWeights) {
    auto regCosts = makeRegCosts(w, mapper, settings, b);
    auto costs = transduce(Array<Array<Cost::Ptr>>{
      dataCosts, regCosts
    }, trCat(), IntoArray<Cost::Ptr>());
    solution = solve(birls, costs, solution.X);

    if (!solution.OK()) {
      LOG(ERROR) << "Curve filter failed";
      return Results();
    }
    inlierPositions = getInlierPositions(positionCosts, solution.X);
    if (inlierPositions.size() < settings.minimumInlierCount) {
      LOG(ERROR) << "Not enough inlier positions";
      return Results();
    }
  }
  if (settings.postReg.defined()) {
    solution = BandedIrls::constantSolve(birls,
        transduce(Array<Array<Cost::Ptr>>{
          dataCosts,
          makeRegCosts(settings.postReg.get(), mapper,
              settings, b)
    }, trCat(), IntoArray<Cost::Ptr>()));
    if (!solution.OK()) {
      LOG(ERROR) << "Post reg failed";
      return Results();
    }
  }

  auto reliableSpan = getReliableSpan(inlierPositions);
  auto reliableIndices = mapToIndexSpan(mapper, reliableSpan);
  if (!reliableIndices.initialized() || reliableIndices.width() == 0) {
    LOG(ERROR) << "No reliable data";
    return Results();
  }

  return Results{
    solution,
    positionCosts.size(),
    inlierPositions,
    mapper,
    b, solution.X,
    reliableIndices,
  };
}

Results::Curve Results::curve() const {
  return OK()? Results::Curve(
          timeMapper,
          SplineCurve(
              basis.basis,
              X.sliceRowsTo(timeMapper.sampleCount())), 1.0_m,
              reliableIndices) : Curve();
}



}
} /* namespace sail */
