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
#include <server/nautical/WGS84.h>
#include <server/common/DiscreteOutlierFilter.h>
#include <server/math/nonlinear/SpatialMedian.h>
#include <server/math/SplineUtils.h>
#include <server/common/TimedValueUtils.h>
#include <server/nautical/GeographicReference.h>

namespace sail {
namespace SplineGpsFilter {

typedef RawSplineBasis<double, 3>::Weights Weights;

OutlierRejector::Settings Settings::positionSettings() const {
  OutlierRejector::Settings settings;
  settings.sigma = inlierThreshold.meters();
  settings.initialAlpha = 1.0e6;
  settings.initialBeta = 0.001;
  return settings;
}


void EcefCurve::initializeMotionBasis() {
  _motionBasis = _basis.derivative().scale(
      1.0/_mapper.period.seconds());
}

EcefCurve::EcefCurve(
    const TimeMapper &mapper,
    const SmoothBoundarySplineBasis<double, 3> &b,
    const double *coefs, int stride) : _mapper(mapper), _basis(b) {
  initializeMotionBasis();
  int n = _mapper.sampleCount;
  for (int i = 0; i < 3; i++) {
    _coefs[i] = Array<double>::fill(n, 0.0);
  }
  for (int i = 0; i < n; i++) {
    const double *x = coefs + stride*i;
    for (int j = 0; j < 3; j++) {
      _coefs[j][i] = x[j];
    }
  }
}

EcefCurve::EcefCurve(
      const TimeMapper &mapper,
      const SmoothBoundarySplineBasis<double, 3> &basis,
      const MDArray2d &coefs) : _mapper(mapper),
          _basis(basis) {
  initializeMotionBasis();
  int rows = coefs.rows();
  for (int i = 0; i < 3; i++) {
    _coefs[i] = coefs.sliceCol(i).getStorage().sliceTo(rows);
  }
}

TimeStamp EcefCurve::lower() const {
  return _mapper.unmap(_basis.raw().lowerDataBound());
}

TimeStamp EcefCurve::upper() const {
  return _mapper.unmap(_basis.raw().upperDataBound());
}

bool EcefCurve::covers(TimeStamp t) const {
  return lower() <= t && t <= upper();
}

ECEFCoords<double> EcefCurve::evaluateEcefPosition(TimeStamp t) const {
  auto x = _mapper.mapToReal(t);
  auto unit = 1.0_m;
  return ECEFCoords<double>{
    _basis.evaluate(_coefs[0].ptr(), x)*unit,
    _basis.evaluate(_coefs[1].ptr(), x)*unit,
    _basis.evaluate(_coefs[2].ptr(), x)*unit
  };
}

ECEFCoords<double, 1> EcefCurve::evaluateEcefMotion(TimeStamp t) const {
  auto x = _mapper.mapToReal(t);
  auto unit = 1.0_mps;
  return ECEFCoords<double, 1>{
    _motionBasis.evaluate(_coefs[0].ptr(), x)*unit,
    _motionBasis.evaluate(_coefs[1].ptr(), x)*unit,
    _motionBasis.evaluate(_coefs[2].ptr(), x)*unit
  };
}

Span<TimeStamp> EcefCurve::span() const {
  return Span<TimeStamp>(lower(), upper());
}

GeographicPosition<double> EcefCurve::evaluateGeographicPosition(
    TimeStamp t) const {
  auto pos = evaluateEcefPosition(t);
  auto lla = ECEF::convert(pos);
  return GeographicPosition<double>(lla.lon, lla.lat, lla.height);
}

HorizontalMotion<double> EcefCurve::evaluateHorizontalMotion(
    TimeStamp t) const {
  auto pos = evaluateEcefPosition(t);
  auto m = evaluateEcefMotion(t);
  return ECEF::computeEcefToHMotion(pos, m);
}

const TimeMapper &EcefCurve::timeMapper() const {
  return _mapper;
}

bool EcefCurve::defined() const {
  return _mapper.offset.defined();
}

template <typename T>
Array<bool> computeMask(
    Duration<double> cut,
    std::function<double(TimedValue<T>, TimedValue<T>)> cost,
    double threshold,
    const Array<TimedValue<T>> &samples) {
  DiscreteOutlierFilter::Settings outlierSettings;
   outlierSettings.threshold = threshold;
   typedef TimedValue<T> TV;
   auto mask = DiscreteOutlierFilter::computeOutlierMask<const TV*, TV>(
       samples.begin(), samples.end(), cost,
       [=](TV x, TV y) {
         return y.time - x.time > cut;
       },
       outlierSettings);
   CHECK(mask.size() == samples.size());
   return mask;
}



Array<TimedValue<GeographicPosition<double>>> filterPositions(
    const Array<TimedValue<GeographicPosition<double>>> &positions,
    const Settings &settings) {

  Duration<double> timeReg =
      settings.maxLengthQuantizationError/settings.maxSpeed;

  Array<bool> mask = DiscreteOutlierFilter::identifyOutliers<
      GeographicPosition<double>>(
      positions,
      [&](const TimedValue<GeographicPosition<double>> &a,
          const TimedValue<GeographicPosition<double>> &b) {
        return (distance<double>(a.value, b.value)
            /(fabs(a.time - b.time) + timeReg))
            .metersPerSecond();
      }, settings.timeBackSteps, settings.maxSpeed.metersPerSecond());
  return positions.slice(mask);
}

Array<TimedValue<HorizontalMotion<double>>> filterMotions(
    const Array<TimedValue<HorizontalMotion<double>>> &motions,
    const Settings &settings) {
  auto mps2 = 1.0_m/(1.0_s*1.0_s);

  Duration<double> timeReg =
      settings.maxVelocityQuantizationError/settings.maxAcceleration;
  Array<bool> mask = DiscreteOutlierFilter::identifyOutliers<
      HorizontalMotion<double>>(
          motions,
          [&](const TimedValue<HorizontalMotion<double>> &a,
              const TimedValue<HorizontalMotion<double>> &b) {
    return double(HorizontalMotion<double>(a.value - b.value).norm().metersPerSecond()
            /(fabs(a.time - b.time) + timeReg).seconds());
  }, settings.timeBackSteps, double(settings.maxAcceleration/mps2));
  return motions.slice(mask);
}

struct CurveData {
  Array<TimedValue<GeographicPosition<double>>> positions;
  Array<TimedValue<HorizontalMotion<double>>> motions;
  Span<TimeStamp> timeSpan;
};

Array<Span<TimeStamp>> segmentRelevantTimeSpans(
    const Array<TimeStamp> &positionTimes,
    const Array<TimeStamp> &motionTimes,
    Duration<double> maxGap) {
  auto times = merge(positionTimes, motionTimes);
  auto timeSpans = listTimeSpans(
      times, maxGap, false);

  auto spanPerPosition = getTimeSpanPerTimeStamp(
      timeSpans, positionTimes);
  auto spanMask = Array<bool>::fill(timeSpans.size(), false);
  for (auto i: spanPerPosition) {
    if (i != -1) {
      spanMask[i] = true;
    }
  }
  return timeSpans.slice(spanMask);
}


Array<CurveData> segmentCurveData(const Array<TimedValue<GeographicPosition<double>>> &allPositionData,
    const Array<TimedValue<HorizontalMotion<double>>> &allMotionData,
    const Settings &settings) {
  auto cleanPositions = filterPositions(allPositionData, settings);
  auto cleanMotions = filterMotions(allMotionData, settings);
  auto spans = segmentRelevantTimeSpans(
      getTimes(cleanPositions),
      getTimes(cleanMotions), settings.maxGap);
  auto cutPositions = cutTimedValues(
      cleanPositions.begin(), cleanPositions.end(),
      spans);
  auto cutMotions = cutTimedValues(
      cleanMotions.begin(), cleanMotions.end(),
      spans);
  int spanCount = spans.size();
  CHECK(cutPositions.size() == spanCount);
  CHECK(cutMotions.size() == spanCount);
  CHECK(cutPositions.size() == spanCount);
  ArrayBuilder<CurveData> dst(spanCount);
  for (int i = 0; i < spanCount; i++) {
    auto p = cutPositions[i];
    if (!p.empty()) {
      dst.add(CurveData{
        p,
        cutMotions[i],
        spans[i]
      });
    }
  }
  return dst.get();
}

void addPositionTerms(
    RobustSplineFit<3> *problem,
    const Array<TimedValue<GeographicPosition<double>>> &positions,
    Length<double> sigma) {
  for (auto p: positions) {
    auto y = ECEF::convert(p.value);
    Eigen::Vector3d xyz(
       y.xyz[0].meters(),
       y.xyz[1].meters(),
       y.xyz[2].meters()
    );
    problem->addObservation(p.time, 0, xyz, sigma.meters());
  }
}

void addMotionTerm(
    RobustSplineFit<3> *dst,
    const TimedValue<HorizontalMotion<double>> &motion,
    const TimedValue<GeographicPosition<double>> &position,
    Length<double> sigma) {
  ECEFCoords<double, 1> k = ECEF::hMotionToXYZ<double>(
      ECEF::geo2lla(position.value),
      motion.value);
  Eigen::Vector3d xyz(
      k.xyz[0].metersPerSecond(),
      k.xyz[1].metersPerSecond(),
      k.xyz[2].metersPerSecond());
  //dst->addCost(1, 1.0, motion.time, xyz);
  dst->addObservation(motion.time, 1,
      xyz, sigma.meters());
}

void addMotionTerms(RobustSplineFit<3> *dst,
    const CurveData &data,
    Length<double> sigma) {
  auto positionTimes = getTimes(data.positions);
  CHECK(!positionTimes.empty());
  auto motionTimes = getTimes(data.motions);
  auto closestPositions = findNearestTimePerTime(
      motionTimes, positionTimes);
  for (int i = 0; i < data.motions.size(); i++) {
    addMotionTerm(
        dst,
        data.motions[i],
        data.positions[closestPositions[i]],
        sigma);
  }
}

TimeMapper makeTimeMapper(const CurveData &src,
    Duration<double> period) {
  int n = int(ceil((src.timeSpan.maxv() - src.timeSpan.minv())
      /period));
  return TimeMapper(src.timeSpan.minv(),
      period, n);
}


EcefCurve filterOneCurve3d(const CurveData &src,
    const Settings &settings) {
  CHECK(!src.positions.empty());
  auto mapper = makeTimeMapper(src, settings.samplingPeriod);
  RobustSplineFit<3>::Settings fitSettings;
  fitSettings.wellPosednessOrder = 1;
  fitSettings.wellPosednessReg = settings.wellPosednessReg;
  fitSettings.regOrder = 2;
  fitSettings.regWeight = settings.regWeight;
  fitSettings.ignoreConstantIfVariable = false;
  fitSettings.iters = settings.iters;
  RobustSplineFit<3> problem(mapper, fitSettings);

  // No regularization for 0th order, because there is at l
  // least one position.
  addPositionTerms(&problem, src.positions, settings.inlierThreshold);
  addMotionTerms(&problem, src, settings.inlierThreshold);
  LOG(INFO) << "Solving GPS filtering problem...";
  auto solution = problem.solve();
  LOG(INFO) << "Solved!";
  return EcefCurve(
      mapper,
      problem.basis(),
      solution);
}

Array<double> getCol(const MDArray2d &src, int i) {
  return src.sliceCol(i).getStorage().sliceTo(src.rows());
}

Array<EcefCurve> filterEveryCurve(
    const Array<CurveData> &curveData,
    const Settings &settings) {
  int n = curveData.size();
  Array<EcefCurve> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = filterOneCurve3d(curveData[i], settings);
  }
  return dst;
}

Array<EcefCurve> segmentAndFilter(
    const Array<TimedValue<GeographicPosition<double>>> &allPositionData,
        const Array<TimedValue<HorizontalMotion<double>>> &allMotionData,
        Settings settings) {
  auto curveData = segmentCurveData(allPositionData, allMotionData,
      settings);
  return filterEveryCurve(curveData, settings);
}



}
}
