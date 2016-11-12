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

struct Curve {
  TimeMapper timeMapper;
  SmoothBoundarySplineBasis<double, 3> basis;

  Span<TimeStamp> timeSpan() const {
    return Span<TimeStamp>(
        timeMapper.unmap(basis.raw().lowerDataBound()),
        timeMapper.unmap(basis.raw().upperDataBound()));
  }

  Curve() {}
  Curve(const TimeMapper &mapper) : timeMapper(mapper),
      basis(mapper.sampleCount) {}

};

static const int blockSize = 4;

OutlierRejector::Settings Settings::positionSettings() const {
  OutlierRejector::Settings settings;
  settings.sigma = inlierThreshold.meters();
  settings.initialAlpha = 1.0e6;
  settings.initialBeta = 0.001;
  return settings;
}

Settings::Settings() {
  lmSettings.iters = 8;
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

template <typename T>
void evaluateEcefPos(
    const Weights &weights,
    const T *input,
    T *dst) {
  for (int i = 0; i < 3; i++) {
    dst[i] = T(0.0);
  }
  for (int i = 0; i < weights.dim; i++) {
    if (weights.isSet(i)) {
      int offs = blockSize*weights.inds[i];
      const T *x = input + offs;
      double w = weights.weights[i];
      for (int j = 0; j < 3; j++) {
        dst[j] += x[j]*w;
      }
    }
  }
}

struct DataFitness {
  Weights weights;
  Eigen::Vector3d data;
  OutlierRejector rejector;
  std::function<double(int)> weightToIndex;

  static const int inputCount = blockSize*Weights::dim;
  static const int outputCount = 3;

  static const bool robust = true;

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
    bool evaluateSub(double weight, const T *input, T *output) const {
      for (int i = 0; i < inputCount; i++) {
        CHECK(isFinite(input[i]));
      }
      T xyz[3];
      evaluateEcefPos<T>(weights, input, xyz);
      for (int i = 0; i < 3; i++) {
        output[i] = weight*(xyz[i] - data[i]);
      }
      return true;
    }

  template <typename T>
    bool evaluate(const T *input, T *output) const {
      double weight = robust? rejector.computeWeight() : 1.0;
      return evaluateSub<T>(
          weight, input, output);
    }

  void update(int iteration, const double *input) {
    if (1 <= iteration) {
      auto newWeight = weightToIndex(iteration);

      double tmp[3] = {0, 0, 0};
      if (evaluateSub<double>(1.0, input, tmp)) {
        auto residual = Eigen::Vector3d(tmp[0], tmp[1], tmp[2]).norm();
        std::cout << "Residual " << residual << std::endl;
        rejector.update(newWeight,
            residual);
        std::cout << "  resulting in weight "
            << rejector.computeWeight() << std::endl;
      }
    } else {
      std::cout << "First iteration weight: "
          << rejector.computeWeight() << std::endl;
    }
  }
};

Eigen::Vector3d toMeters(const ECEFCoords<double> &src) {
  return Eigen::Vector3d(
      src.xyz[0].meters(),
      src.xyz[1].meters(),
      src.xyz[2].meters());
}

template <typename T>
T sqrtHuber(T x) {
  static const T zero = MakeConstant<T>::apply(0.0);
  static const T one = MakeConstant<T>::apply(1.0);
  static const T two = MakeConstant<T>::apply(2.0);
  if (x < zero) {
    return -sqrtHuber(-x);
  } else {
    return x < one? x : sqrt(one + two*(x - one));
  }
}

struct HuberDataCost {
  Weights weights;
  Eigen::Vector3d pos;
  double threshold;

  HuberDataCost(const Weights &w,
      const Eigen::Vector3d &p, double t) : weights(w),
          pos(p), threshold(t) {}

  static const int inputCount = blockSize*Weights::dim;
  static const int outputCount = 3;

  template <typename T>
  bool evaluate(const T *input, T *output) const {
    double marg = 1.0e-9;
    T res[3];
    evaluateEcefPos<T>(weights, input, res);
    for (int i = 0; i < 3; i++) {
      res[i] -= T(pos(i));
    }
    auto len = sqrt(marg + sqr(res[0]) + sqr(res[1]) + sqr(res[2]));
    auto h = threshold*sqrtHuber(len/threshold);
    auto f = h/len;
    std::cout << "len = " << len << std::endl;
    std::cout << "h = " << h << std::endl;
    std::cout << "f = " << f << std::endl;
    for (int i = 0; i < 3; i++) {
      output[i] = f*res[i];
    }
    return true;
  }
};

struct SquaredCost {
  Weights weights;
  Eigen::Vector3d pos;

  static const int inputCount = blockSize*Weights::dim;
  static const int outputCount = 3;

  template <typename T>
    bool evaluate(const T *input, T *output) const {
      T res[3];
      evaluateEcefPos<T>(weights, input, output);
      for (int i = 0; i < 3; i++) {
        output[i] -= T(pos(i));
      }
      return true;
    }
};

void addDataTerm(const Settings &settings,
    Span<int> dstSpan,
    const Eigen::Vector3d &observation,
    const RawSplineBasis<double, 3>::Weights &weights,
    BandedLevMar::Problem<double> *dst) {

}

void addPositionDataTerm(
    const Settings &settings,
    const Curve &c,
    const Span<int> &sampleSpan,
    const TimedValue<GeographicPosition<double>> &value,
    BandedLevMar::Problem<double> *dst) {
  double x = c.timeMapper.mapToReal(value.time);
  auto weights = c.basis.build(x);
  auto dstSpan = weights.getSpanAndOffsetAt0();
  auto pos = ECEF::convert(value.value);
  dst->addCostFunction(blockSize*(dstSpan + sampleSpan.minv()),
      new SquaredCost{weights, toMeters(pos)});
}

auto timeMarg = 0.0001_s;

double computeSpeed(
    const TimedValue<GeographicPosition<double>> &a,
    const TimedValue<GeographicPosition<double>> &b) {
  auto dif = b.time - a.time + timeMarg;
  return (distance(a.value, b.value)/dif).metersPerSecond();
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

void addPositionDataTerms(
    const Settings &settings,
    const Curve &c,
    Span<int> sampleSpan,
    const Array<TimedValue<GeographicPosition<double>>> &pd,
    BandedLevMar::Problem<double> *dst) {
  auto mask = computeMask<GeographicPosition<double>>(
      settings.outlierCutThreshold, &computeSpeed,
      settings.maxSpeed.metersPerSecond(), pd);
  for (int i = 0; i <pd.size(); i++) {
    if (mask[i]) {
      addPositionDataTerm(settings, c,
          sampleSpan, pd[i], dst);
    }
  }
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
  fitSettings.iters = 30;
  RobustSplineFit<3> problem(mapper, fitSettings);

  // No regularization for 0th order, because there is at l
  // least one position.
  addPositionTerms(&problem, src.positions, settings.inlierThreshold);
  addMotionTerms(&problem, src, settings.inlierThreshold);
  auto solution = problem.solve();
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
