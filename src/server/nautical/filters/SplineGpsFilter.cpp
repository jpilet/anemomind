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

EcefCurve::EcefCurve(
    const TimeMapper &mapper,
    const SmoothBoundarySplineBasis<double, 3> &b,
    const double *coefs, int stride) : _mapper(mapper), _basis(b),

        // f(x) = g(x/T);
        // f'(x) = g'(x/T)*(1/T)
        _motionBasis(b.derivative().scale(1.0/mapper.period.seconds())) {

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

template <typename T, int n>
struct ToEcef {};

template <typename T>
struct ToEcef<T, 0> {
  static Length<T> apply(T x) {return Length<T>::meters(T(x));}
};
template <typename T>
struct ToEcef<T, 1> {
  static Velocity<T> apply(T x) {return Velocity<T>::metersPerSecond(T(x));}
};

template <typename T, int n>
ECEFCoords<T, n> toEcefCoords(T *x) {
  ECEFCoords<T, n> dst;
  for (int i = 0; i < 3; i++) {
    dst.xyz[i] = ToEcef<T, n>::apply(x[i]);
  }
  return dst;
}

template <typename T>
double onlyScalar(T x) {
  return x.a;
}

template <>
double onlyScalar(double x) {
  return x;
}

struct MotionDataTerm {
  Weights position, motion;
  HorizontalMotion<double> dst;
  Duration<double> period;

  static const int inputCount = blockSize*Weights::dim;
  static const int outputCount = 3;

  MotionDataTerm(
      Weights p, Weights m,
      const HorizontalMotion<double> &v,
      Duration<double> per) : position(p),
          motion(m), dst(v), period(per) {}

  template <typename T>
  bool evaluate(const T *input, T *output) const {
    T pos[3], mot[3];
    evaluateEcefPos<T>(position, input, pos);
    evaluateEcefPos<T>(motion, input, mot);
    auto hm = ECEF::computeNorthEastDownMotion<T>(
        toEcefCoords<T, 0>(pos),
        toEcefCoords<T, 1>(mot));
    if (isFinite(hm[0]) && isFinite(hm[1])) {
      for (int i = 0; i < 2; i++) {
        auto k = T(period.seconds()*dst[i].metersPerSecond());
        int flipped = 1 - i;
        output[i] = hm(flipped) - k;
      }
      output[2] = hm(2);
    } else {
      for (int i = 0; i < 3; i++) {
        output[i] = T(100.0);
      }
    }
    return true;
  }
};

void addMotionDataTerm(
    const Settings &settings,
    const SmoothBoundarySplineBasis<double, 3> &positionBasis,
    const SmoothBoundarySplineBasis<double, 3> &motionBasis,
    const TimeMapper &mapper,
    Span<int> sampleSpan,
    const TimedValue<HorizontalMotion<double>> &value,
    BandedLevMar::Problem<double> *dst) {
  auto x = mapper.mapToReal(value.time);
  auto positionWeights = positionBasis.build(x);
  auto span = positionWeights.getSpanAndOffsetAt0();
  auto motionWeights = motionBasis.build(x);
  motionWeights.shiftTo(span.minv());
  dst->addCostFunction(blockSize*(span + sampleSpan.minv()), new MotionDataTerm(
      positionWeights, motionWeights,
      value.value, mapper.period));
}

double computeAcceleration(
    const TimedValue<HorizontalMotion<double>> &a,
    const TimedValue<HorizontalMotion<double>> &b) {
  return HorizontalMotion<double>(a.value
      - b.value).norm().metersPerSecond()
      /(b.time - a.time + timeMarg).seconds();
}

void addMotionDataTerms(
    const Settings &settings,
    const Curve &curve,
    Span<int> sampleSpan,
    const Array<TimedValue<HorizontalMotion<double>>> &pm,
    BandedLevMar::Problem<double> *dst) {
  auto der = curve.basis.derivative();

  auto mask = computeMask<HorizontalMotion<double>>(
      settings.outlierCutThreshold,
      &computeAcceleration,
      settings.maxAcceleration/(1.0_mps/1.0_s),
      pm);

  for (int i = 0; i < pm.size(); i++) {
    if (mask[i]) {
      addMotionDataTerm(settings, curve.basis, der, curve.timeMapper,
          sampleSpan, pm[i],
          dst);
    }
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
    evaluateEcefPos<T>(weights, input, output);
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
    auto span = weights.getSpanAndOffsetAt0();
    dst->addCostFunction(blockSize*(span + sampleSpan.minv()),
        new RegCost(weights*s.regWeight));
  }
}

struct Stabilizer {
  double w = 1.0e-12;

  Stabilizer(double p) : w(p) {}

  static const int inputCount = blockSize;
  static const int outputCount = blockSize;

  template <typename T>
  bool evaluate(const T *input, T *output) const {
    for (int i = 0; i < blockSize; i++) {
      output[i] = w*input[i];
    }
    return true;
  }

};

void addStabilizeTerms(
    const Settings &settings,
    Span<int> sampleSpan,
    BandedLevMar::Problem<double> *dst) {
  for (auto i: sampleSpan) {
    dst->addCostFunction(blockSize*Span<int>(i, i+1),
        new Stabilizer(settings.stabilizerWeight));
  }
}

Array<Eigen::Vector3d>
  to3dPositions(
      const Array<TimedValue<GeographicPosition<double>>> &positions) {
  int n = positions.size();
  Array<Eigen::Vector3d> data(n);
  for (int i = 0; i < n; i++) {
    auto k = ECEF::convert(positions[i].value);
    data[i] = Eigen::Vector3d(
        k.xyz[0].meters(),
        k.xyz[1].meters(),
        k.xyz[2].meters());
  }
  return data;
}

void initializeByMedian(
    const Curve &curve,
    const Array<TimedValue<GeographicPosition<double>>> &src,
    double *dst) {
  SpatialMedian::Settings settings;
  auto pos3d = to3dPositions(src);
  auto medianPos = SpatialMedian::compute<3, double>(
      pos3d, settings);
  Arrayd coefs[3];
  for (int i = 0; i < 3; i++) {
    coefs[i] = fitSplineCoefs(curve.basis,
        [=](int) {return medianPos(i);});
  }
  int n = curve.basis.coefCount();
  for (int i = 0; i < n; i++) {
    int offset = i*blockSize;
    for (int j = 0; j < 3; j++) {
      dst[offset + j] = coefs[j][i];
    }
  }
}

void buildProblemPerCurve(
    const Settings &settings,
    const Curve &c,
    Span<int> sampleSpan,
    const Array<TimedValue<GeographicPosition<double>>> &pd,
    const Array<TimedValue<HorizontalMotion<double>>> &md,
    BandedLevMar::Problem<double> *dst,
    double *Xinit) {
  initializeByMedian(c, pd, Xinit);
  Span<int> valueSpan = blockSize*sampleSpan;

  addPositionDataTerms(settings, c, sampleSpan, pd, dst);
  addMotionDataTerms(settings, c, sampleSpan, md, dst);
  addDataRegTerms(settings, c, sampleSpan, dst);
}

void buildProblem(
    const Settings &settings,
    const Array<Curve> &curves,
    const Array<Span<int>> &sampleSpans,
    const Array<Span<TimeStamp>> &timeSpans,
    const Array<TimedValue<GeographicPosition<double>>> &positionData,
    const Array<TimedValue<HorizontalMotion<double>>> &motionData,
    BandedLevMar::Problem<double> *dst,
    double *Xinit) {
  auto pd = cutTimedValues(
      positionData.begin(), positionData.end(), timeSpans);
  auto md = cutTimedValues(
      motionData.begin(), motionData.end(), timeSpans);
  for (int i = 0; i < curves.size(); i++) {
    auto sampleSpan = sampleSpans[i];
    buildProblemPerCurve(
        settings,
        curves[i],
        sampleSpan,
        pd[i], md[i], dst,
        Xinit + blockSize*sampleSpan.minv());
  }
}

Array<EcefCurve> filter(
    const Array<TimedValue<GeographicPosition<double>>> &positionData,
    const Array<TimedValue<HorizontalMotion<double>>> &motionData,
    const Array<TimeMapper> &segments,
    Settings settings) {
  auto curves = allocateCurves(segments);
  int totalSampleCount = computeTotalSampleCount(segments);
  int dim = blockSize*totalSampleCount;

  BandedLevMar::Problem<double> problem;
  auto sampleSpans = listSampleSpans(curves);
  CHECK(sampleSpans.size() == curves.size());
  CHECK(sampleSpans.last().maxv() == totalSampleCount);
  auto timeSpans = listTimeSpans(curves);

  Eigen::VectorXd Xinit = Eigen::VectorXd::Zero(
      blockSize*totalSampleCount);
  buildProblem(
      settings,
      curves, sampleSpans,timeSpans,
      positionData, motionData, &problem,
      Xinit.data());

  Eigen::VectorXd X = Eigen::VectorXd::Zero(problem.paramCount());
  X.block(0, 0, Xinit.size(), 1) = Xinit;


  BandedLevMar::runLevMar(settings.lmSettings,
      problem, &X);

  Array<EcefCurve> resultCurves(curves.size());
  for (int i = 0; i < curves.size(); i++) {
    auto span = blockSize*sampleSpans[i];
    auto c = curves[i];
    resultCurves[i] = EcefCurve(
        c.timeMapper, c.basis, X.data() + span.minv(), blockSize);
  }

  return resultCurves;
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

template <typename T>
void accumulateTimeStamps(ArrayBuilder<TimeStamp> *dst,
    const Array<TimedValue<T>> &src) {
  for (auto x: src) {
    dst->add(x.time);
  }
}

Array<int> listGaps()

Array<TimeStamp> listAllTimes(
    const Array<TimedValue<GeographicPosition<double>>> &positions,
    const Array<TimedValue<HorizontalMotion<double>>> &motions) {
  ArrayBuilder<TimeStamp> dst(positions.size() + motions.size());
  accumulateTimeStamps(&dst, positions);
  accumulateTimeStamps(&dst, motions);
  return dst.get();
}

Array<EcefCurve> filterAndSegment(
    const Array<TimedValue<GeographicPosition<double>>> &allPositionData,
    const Array<TimedValue<HorizontalMotion<double>>> &allMotionData,
    Settings settings) {
  auto cleanPositions = filterPositions(allPositionData, settings);
  auto cleanMotions = filterMotions(allMotionData, settings);

  auto times = listAllTimes(cleanPositions, cleanMotions);

  // Split into smaller curves

  // Filter all curves, together, in closed form.

  // Return

}



}
}
