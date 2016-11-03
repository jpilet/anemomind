/*
 * SplineGpsFilter.cpp
 *
 *  Created on: 31 Oct 2016
 *      Author: jonas
 */

#include "SplineGpsFilter.h"
#include <server/common/ArrayBuilder.h>
#include <server/common/TimedValueCutter.h>
#include <server/math/OutlierRejector.h>
#include <server/common/LineKM.h>

namespace sail {
namespace SplineGpsFilter {

static const int blockSize = 4;

OutlierRejector::Settings Settings::positionSettings() const {
  OutlierRejector::Settings settings;
  settings.sigma = inlierThreshold.meters();
  return settings;
}

Array<Curve> allocateCurves(const Array<TimeMapper> &mappers) {
  int n = mappers.size();
  Array<Curve> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = Curve(mappers[i]);
  }
  return dst;
}

int computeTotalSampleCount(const Array<TimeMapper> &mappers) {
  int n = 0;
  for (auto m: mappers) {
    n += m.sampleCount;
  }
  return n;
}

Array<Span<int>> listSampleSpans(const Array<Curve> &curves) {
  int n = curves.size();
  Array<Span<int>> dst(n);
  int offset = 0;
  for (int i = 0; i < curves.size(); i++) {
    int next = offset + curves[i].timeMapper.sampleCount;
    dst[i] = Span<int>(offset, next);
    offset = next;
  }
  return dst;
}

Array<Span<TimeStamp>> listTimeSpans(const Array<Curve> &curves) {
  int n = curves.size();
  Array<Span<TimeStamp>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = curves[i].timeSpan();
  }
  return dst;
}


std::function<double(int)> makeWeightToIndex(int iters, double from, double to) {
  LineKM line(0, iters, log(from), log(to));
  return [=](int i) {
    return exp(line(i));
  };
}

struct DataFitness {
  typedef RawSplineBasis<double, 3>::Weights Weights;
  Weights weights;
  Eigen::Vector3d data;
  OutlierRejector rejector;
  std::function<double(int)> weightToIndex;

  static const int inputCount = blockSize*Weights::dim;
  static const int outputCount = 3;

  DataFitness(
      const RawSplineBasis<double, 3>::Weights &w,
      const Eigen::Vector3d &d,
      const OutlierRejector::Settings &s,
      std::function<double(int)> w2i) :
        weights(w),
        data(d),
        weightToIndex(w2i),
        rejector(OutlierRejector(s)) {}

  template <typename T>
  bool evaluate(const T *input, T *output) const {
    T xyz[3] = {T(0.0), T(0.0), T(0.0)};
    for (int i = 0; i < Weights::dim; i++) {
      const T *x = input + blockSize*weights.inds[i];
      double w = weights.weights[i];
      for (int j = 0; j < 3; j++) {
        xyz[j] += x[j]*w;
      }
    }
    double rw = rejector.computeWeight();
    for (int i = 0; i < 3; i++) {
      output[i] = rw*(xyz[i] - data[i]);
    }
    return true;
  }

  void update(int iteration, const double *r3) {
    rejector.update(weightToIndex(iteration), Eigen::Vector3d(
        r3[0], r3[1], r3[2]).norm());
  }
};

Eigen::Vector3d toMeters(const ECEFCoords<double> &src) {
  return Eigen::Vector3d(
      src.xyz[0].meters(),
      src.xyz[1].meters(),
      src.xyz[2].meters());
}

std::function<double(int)> Settings::weightToIndex() const {
  return makeWeightToIndex(lmSettings.iters, initialWeight, finalWeight);
}

void addDataTerm(const Settings &settings,
    Span<int> sampleSpan,
    const Eigen::Vector3d &observation,
    const RawSplineBasis<double, 3>::Weights &weights,
    std::function<double(int)> w2i,
    BandedLevMar::Problem<double> *dst) {
  auto f = new DataFitness(weights, observation,
            settings.positionSettings(), w2i);
  auto cost = dst->addCostFunction(
      blockSize*(weights.span() + sampleSpan.minv()), f);
  cost->addIterationCallback([&](int i, const double *residuals) {
    f->update(i, residuals);
  });
}

void addPositionDataTerm(
    const Settings &settings,
    const Curve &c,
    const Span<int> &sampleSpan,
    const TimedValue<GeographicPosition<double>> &value,
    std::function<double(int)> w2i,
    BandedLevMar::Problem<double> *dst) {
  double x = c.timeMapper.mapToReal(value.time);
  auto weights = c.basis.build(x);
  auto pos = ECEF::convert(value.value);
  addDataTerm(settings, sampleSpan,
      toMeters(pos), weights, w2i, dst);
}

void addPositionDataTerms(
    const Settings &settings,
    const Curve &c,
    Span<int> sampleSpan,
    const Array<TimedValue<GeographicPosition<double>>> &pd,
    BandedLevMar::Problem<double> *dst) {
  auto w2i = settings.weightToIndex();
  for (auto sample: pd) {
    addPositionDataTerm(settings, c, sampleSpan, sample, w2i, dst);
  }
}

void addMotionDataTerm(
    const Settings &settings,
    const SmoothBoundarySplineBasis<double, 3> &basis,
    const TimeMapper &mapper,
    Span<int> sampleSpan, const TimedValue<HorizontalMotion<double>> &value,
    const std::function<double(int)> w2i, BandedLevMar::Problem<double> *dst) {
  auto x = mapper.mapToReal(value.time);
  auto weights = basis.build(x);
}

void addMotionDataTerms(
    const Settings &settings,
    const Curve &curve,
    Span<int> sampleSpan,
    const Array<TimedValue<HorizontalMotion<double>>> &pm,
    BandedLevMar::Problem<double> *dst) {
  auto der = curve.basis.derivative();
  auto w2i = settings.weightToIndex();
  for (auto sample: pm) {
    addMotionDataTerm(settings, der, curve.timeMapper,
        sampleSpan, sample, w2i,
        dst);
  }
}

struct RegCost {
  typedef RawSplineBasis<double, 3>::Weights Weights;

  Weights weights;

  static const int inputCount = blockSize*Weights::dim;
  static const int outputCount = 3;

  RegCost(Weights w) : weights(w) {}

  template <typename T>
  bool evaluate(const T *input, T *output) const {
    T dst[3] = {T(0.0), T(0.0), T(0.0)};
    for (int i = 0; i < Weights::dim; i++) {
      auto x = input + blockSize*i;
      double w = weights.weights[i];
      for (int j = 0; j < 3; j++) {
        dst[j] += x[j]*w;
      }
    }
    for (int i = 0; i < 3; i++) {
      output[i] = dst[i];
    }
    return true;
  }
};

void addDataRegTerms(const Settings &s,
    const Curve &c,
    Span<int> sampleSpan,
    BandedLevMar::Problem<double> *dst) {
  auto der2 = c.basis.derivative().derivative();
  for (int i = 0; i < c.timeMapper.sampleCount; i++) {
    auto weights = der2.build(i);
    int from = sampleSpan.minv();
    int to = from + weights.dim;
    dst->addCostFunction(blockSize*Spani(from, to), new RegCost(weights*s.regWeight));
  }
}

void buildProblemPerCurve(
    const Settings &settings,
    const Curve &c,
    Span<int> sampleSpan,
    const Array<TimedValue<GeographicPosition<double>>> &pd,
    const Array<TimedValue<HorizontalMotion<double>>> &md,
    BandedLevMar::Problem<double> *dst) {
  Span<int> valueSpan = blockSize*sampleSpan;

  // Add data terms
  addPositionDataTerms(settings, c, sampleSpan, pd, dst);

  // addMotionDataTerms
  addMotionDataTerms(settings, c, sampleSpan, md, dst);

  // addRegDataTerms
  addDataRegTerms(settings, c, sampleSpan, dst);

  // addStabilizeDataTerms

}

void buildProblem(
    const Settings &settings,
    const Array<Curve> &curves,
    const Array<Span<int>> &sampleSpans,
    const Array<Span<TimeStamp>> &timeSpans,
    const Array<TimedValue<GeographicPosition<double>>> &positionData,
    const Array<TimedValue<HorizontalMotion<double>>> &motionData,
    BandedLevMar::Problem<double> *dst) {
  auto pd = cutTimedValues(
      positionData.begin(), positionData.end(), timeSpans);
  auto md = cutTimedValues(
      motionData.begin(), motionData.end(), timeSpans);
  for (int i = 0; i < curves.size(); i++) {
    buildProblemPerCurve(
        settings,
        curves[i],
        sampleSpans[i],
        pd[i], md[i], dst);
  }
}

Array<Curve> filter(
    const Array<TimedValue<GeographicPosition<double>>> &positionData,
    const Array<TimedValue<HorizontalMotion<double>>> &motionData,
    const Array<TimeMapper> &segments,
    const Settings &settings) {
  auto curves = allocateCurves(segments);
  int totalSampleCount = computeTotalSampleCount(segments);
  int dim = blockSize*totalSampleCount;

  BandedLevMar::Settings lmSettings;
  BandedLevMar::Problem<double> problem;
  auto X = Eigen::VectorXd::Zero(dim);
  auto sampleSpans = listSampleSpans(curves);
  CHECK(sampleSpans.size() == curves.size());
  CHECK(sampleSpans.last().maxv() == totalSampleCount);
  auto timeSpans = listTimeSpans(curves);
  buildProblem(
      settings,
      curves, sampleSpans,timeSpans,
      positionData, motionData, &problem);

  return curves;
}



}
}
