/*
 * CurrentCalib.cpp
 *
 *  Created on: Apr 15, 2016
 *      Author: jonas
 */

#include <server/nautical/calib/CurrentCalib.h>
#include <server/common/TimeStamp.h>
#include <server/nautical/calib/CornerCalib.h>
#include <server/nautical/filters/TimedAngleIntegrator.h>
#include <server/nautical/common.h>
#include <server/transducers/Transducer.h>

namespace sail {
namespace CurrentCalib {

Settings::Settings() : windowSize(Duration<double>::minutes(2.0)),
    samplingPeriod(Duration<double>::seconds(2.0)),
    debugShowScatter(false) {}

namespace {
  struct CurrentDataSample {
    TimeStamp time;
    Duration<double> maxDuration;
    HorizontalMotion<double> gpsMotion, rawBoatOverWater;

    HorizontalMotion<double> refMotion() const {
      return gpsMotion;
    }
  };

  template <typename T>
  HorizontalMotion<T> rotate(T rads, const HorizontalMotion<T> &xy) {
    auto c = cos(rads);
    auto s = sin(rads);
    return HorizontalMotion<T>(c*xy[0] - s*xy[1], s*xy[0] + c*xy[1]);
  }

  struct CurrentCorrector {
    template <typename T>
    HorizontalMotion<T> evalFlow(const CurrentDataSample &s, const T *params) const {
      typedef Eigen::Matrix<T, 2, 1> Vec;
      T bias = params[0];
      auto speedOffset = T(params[1])*CornerCalib::getUnit<T>();
      T angleOffset = params[2];

      return computeCurrentFromBoatMotion<HorizontalMotion<T> >(
          bias*rotate<T>(angleOffset, s.rawBoatOverWater.cast<T>() +
              CornerCalib::setNorm<T>(s.rawBoatOverWater.cast<T>(),
                  speedOffset)), s.gpsMotion.cast<T>());

    }

    Arrayd initialParams() const {
      return Arrayd{1.0, 0.0, 0.0};
    }

    HorizontalMotion<double> evalHorizontalMotion(const CurrentDataSample &s,
        const Arrayd &params) const {
      assert(!params.empty());
      return evalFlow<double>(s, params.ptr());
    }
  };

  template <DataCode Code>
  TimedValueIntegrator<typename TypeForCode<Code>::type> makeIntegrator(
      const NavDataset &ds) {
    auto samples = ds.samples<Code>();
    return TimedValueIntegrator<typename TypeForCode<Code>::type>::make(
        samples.begin(), samples.end());
  }

  Array<CurrentDataSample> makeSamples(
      const NavDataset &ds,
      const Duration<double> &samplingPeriod) {
    TimeStamp lower = ds.lowerBound();
    if (lower.undefined()) {
      LOG(ERROR) << "Failed to sample because lower time bound undefined";
      return Array<CurrentDataSample>();
    }


    TimeStamp upper = ds.upperBound();
    if (upper.undefined()) {
      LOG(ERROR) << "Failed to sample because upper time bound undefined";
      return Array<CurrentDataSample>();
    }

    assert(lower <= upper);

#define MAKE_NONEMPTY_INTEGRATOR(varName, code) \
auto varName = makeIntegrator<code>(ds); \
if (varName.empty()) { \
  LOG(ERROR) << "Failed to sample because " << #code << " is empty"; \
  return Array<CurrentDataSample>(); \
}
    MAKE_NONEMPTY_INTEGRATOR(gpsBearing, GPS_BEARING)
    MAKE_NONEMPTY_INTEGRATOR(gpsSpeed, GPS_SPEED)
    MAKE_NONEMPTY_INTEGRATOR(magHeading, MAG_HEADING)
    MAKE_NONEMPTY_INTEGRATOR(watSpeed, WAT_SPEED)
#undef MAKE_NONEMPTY_INTEGRATOR

    int n = int(floor((upper - lower)/samplingPeriod));

    auto half = 0.5*samplingPeriod;

    ArrayBuilder<CurrentDataSample> samples(n);
    for (int i = 0; i < n; i++) {
      auto from = lower + double(i)*samplingPeriod;
      auto to = from + samplingPeriod;

      auto rawWatSpeed0 = watSpeed.computeAverage(from, to);
      if (rawWatSpeed0.defined()) {
        auto rawWatSpeed = rawWatSpeed0.get();
        auto rawMagHeading0 = magHeading.computeAverage(from, to);
        if (rawMagHeading0.defined()) {
          auto rawMagHeading = rawMagHeading0.get();
          auto rawGpsBearing0 = gpsBearing.computeAverage(from, to);
          if (rawGpsBearing0.defined()) {
            auto rawGpsBearing = rawGpsBearing0.get();
            auto rawGpsSpeed0 = gpsSpeed.computeAverage(from, to);
            if (rawGpsSpeed0.defined()) {
              auto rawGpsSpeed = rawGpsSpeed0.get();
              auto maxDuration = std::max(
                  std::max(rawGpsSpeed.duration, rawGpsBearing.duration),
                  std::max(rawWatSpeed.duration, rawMagHeading.duration));
              auto middle = from + half;
              samples.add(CurrentDataSample{
                middle,
                maxDuration,
                HorizontalMotion<double>::polar(rawGpsSpeed.value, rawGpsBearing.value),
                HorizontalMotion<double>::polar(rawWatSpeed.value, rawMagHeading.value)
              });
            }
          }
        }
      }
    }
    return samples.get();
  }


  Array<Array<CurrentDataSample> > makeSampleGroups(
        const Array<NavDataset> &ds,
        const Duration<double> &samplingPeriod) {
    return transduce(ds, trMap([&](const NavDataset &ds) {
      return makeSamples(ds, samplingPeriod);
    }), IntoArray<Array<CurrentDataSample>>());
  }

  MotionSamples applyCurrentCalibration(
      const Arrayd &params, const Array<CurrentDataSample> &samples) {
    int n = samples.size();
    CurrentCorrector corr;
    MotionSamples dst(n);
    for (int i = 0; i < n; i++) {
      auto sample = samples[i];
      dst[i] = TimedValue<HorizontalMotion<double> >(
          sample.time, corr.evalHorizontalMotion(sample, params));
    }
    return dst;
  }
}

Array<MotionSamples> computeCorrectedCurrent(
    const Array<NavDataset> &ds, const Settings &s) {
  //auto samples = makeSamples(ds, s.samplingPeriod);
  auto sampleGroups = makeSampleGroups(ds, s.samplingPeriod);

  CornerCalib::Settings cornerSettings;
  cornerSettings.windowSize = int(floor(s.windowSize/s.samplingPeriod));

  auto params = CornerCalib::optimizeCornernessForGroups(CurrentCorrector(),
      sampleGroups, cornerSettings);

  if (params.empty()) {
    LOG(ERROR) << "Calibration failed";
    return Array<MotionSamples>();
  }
  int n = sampleGroups.size();

  Array<MotionSamples> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = applyCurrentCalibration(params, sampleGroups[i]);
  }
  return dst;
}



}
}

