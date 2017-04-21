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
#include <server/common/String.h>
#include <server/math/SampleUtils.h>


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


Array<Span<TimeStamp>> Results::segmentSpans(
    Duration<double> margin) const {
  if (!OK()) {
    return Array<Span<TimeStamp>>();
  }
  ArrayBuilder<TimedValue<bool>> maskBuilder(regCosts.size());
  for (auto t: inlierPositionTimes) {
    maskBuilder.add(TimedValue<bool>(t, true));
  }
  for (auto regCost: regCosts) {
    auto time = regCost.getTime();
    if (time.defined()) {
      maskBuilder.add(TimedValue<bool>(time, regCost.isInlier(X)));
    }
  }
  auto mask = maskBuilder.get();
  return SampleUtils::makeGoodSpans(mask, margin, margin);
}

namespace {
  Span<int> spanToInt(const Span<TimeStamp>& src, const TimeMapper& m) {
    if (!src.initialized()) {
      return Span<int>();
    }
    int from = m.toIntegerIndex(src.minv());
    int to = m.toIntegerIndex(src.maxv());
    return from < to? Span<int>(from, to) : Span<int>();
  }

  Array<Results::Curve> sliceCurveBySpans(
      const Results::Curve& c, const Array<Span<TimeStamp>>& spans) {
    const auto& m = c.timeMapper();
    ArrayBuilder<Results::Curve> dst(spans.size());
    for (auto s: spans) {
      auto si = spanToInt(s, m);
      if (si.initialized()) {
        dst.add(c.slice(si));
      }
    }
    return dst.get();
  }
}

Array<Results::Curve> Results::segmentCurves(Duration<double> margin) const {
  return sliceCurveBySpans(curve(), segmentSpans(margin));
}

Span<TimeStamp> getReliableSpan(
    const Array<TimeStamp>& inlierPositionTimes) {
  return inlierPositionTimes.empty()?
      Span<TimeStamp>() : Span<TimeStamp>(
          inlierPositionTimes.first(),
          inlierPositionTimes.last());
}


bool Settings::robustRegularization() const {
  return regSigma > 0.0_mps2;
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
          settings.inlierThreshold.meters()));
    }
  }
  return costs.get();
}

Cost::Ptr regToCommonCost(const RegCost& x) {
  return x.cost;
}

Array<Cost::Ptr> toCommonCosts(const Array<RegCost>& src) {
  return map(src, &regToCommonCost);
}


Array<RegCost> makeRegCosts(
    double weight,
    const TimeMapper& mapper,
    const Settings& settings,
    const BasisData& b) {
  ExponentialWeighting weights(settings.iterations,
      settings.initialWeight, settings.finalWeight);

  ArrayBuilder<RegCost> costs(mapper.sampleCount());
  auto sigma = (weight/settings.regWeights.last())*(settings.regSigma
                *mapper.period()*mapper.period()).meters();
  for (int i = 0; i < mapper.sampleCount(); i++) {
    CoefsWithOffset<Basis> coefs(b.acceleration.build(i));
    if (settings.robustRegularization()) {
      costs.add(RegCost(std::make_shared<DataCost>(
          coefs.offset, weight*coefs.coefs,
          mat1x2(0, 0),
          weights, sigma)));
    } else {
      costs.add(RegCost(Cost::Ptr(new StaticCost<1, BasisData::dim, 2>(
          coefs.offset, weight*coefs.coefs, mat1x2(0, 0)))));
    }
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

int countInliers(
    const Array<RegCost>& costs,
    const MDArray2d& X) {
  int counter = 0;
  for (auto c: costs) {
    if (c.maybeDataCost) {
      counter += c.maybeDataCost->isInlier(X);
    } else {
      //counter++; // Not sure if this is a good default.
    }
  }
  return counter;
}

void outputReport(
    const Settings& settings,
    const Array<RegCost>& regCosts,
    const MDArray2d& X) {
  DOM::Node output = settings.output;
  if (output.defined()) {
    if (settings.robustRegularization()) {
      auto inlierCount = countInliers(regCosts, X);
      if (inlierCount < regCosts.size()) {
        addSubTextNode(&output, "pre",
            stringFormat("Number of inlier reg terms: %d/%d",
                inlierCount,
                regCosts.size()));
      }
    }
  }
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
  Array<RegCost> lastRegCosts;
  for (auto w: settings.regWeights) {
    auto regCosts = makeRegCosts(w, mapper, settings, b);
    auto costs = concat(Array<Array<Cost::Ptr>>{
      dataCosts, toCommonCosts(regCosts)
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
    lastRegCosts = regCosts;
  }
  auto reliableSpan = getReliableSpan(inlierTimes);
  auto reliableIndices = mapToIndexSpan(mapper, reliableSpan);
  if (!reliableIndices.initialized() || reliableIndices.width() == 0) {
    LOG(ERROR) << "No reliable data";
    return Results();
  }

  outputReport(settings, lastRegCosts, solution.X);

  return Results{
    solution,
    positionCosts.size(),
    inlierTimes,
    mapper,
    b, solution.X,
    reliableIndices,
    lastRegCosts
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
