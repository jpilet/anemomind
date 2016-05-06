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
#include <server/nautical/calib/Correction.h>

namespace sail {
namespace CurrentCalib {

using namespace sail::Correction;

Settings::Settings() : windowSize(Duration<double>::minutes(2.0)),
    samplingPeriod(Duration<double>::seconds(2.0)),
    debugShowScatter(false) {}

namespace {

  template <DataCode Code>
  TimedValueIntegrator<typename TypeForCode<Code>::type> makeIntegrator(
      const NavDataset &ds) {
    auto samples = ds.samples<Code>();
    return TimedValueIntegrator<typename TypeForCode<Code>::type>::make(
        samples.begin(), samples.end());
  }

  Array<RawNav> makeSamples(
      const NavDataset &ds,
      const Duration<double> &samplingPeriod) {
    TimeStamp lower = ds.lowerBound();
    if (lower.undefined()) {
      LOG(ERROR) << "Failed to sample because lower time bound undefined";
      return Array<RawNav>();
    }


    TimeStamp upper = ds.upperBound();
    if (upper.undefined()) {
      LOG(ERROR) << "Failed to sample because upper time bound undefined";
      return Array<RawNav>();
    }

    assert(lower <= upper);

#define MAKE_NONEMPTY_INTEGRATOR(varName, code) \
auto varName = makeIntegrator<code>(ds); \
if (varName.empty()) { \
  LOG(ERROR) << "Failed to sample because " << #code << " is empty"; \
  return Array<RawNav>(); \
}
    MAKE_NONEMPTY_INTEGRATOR(gpsBearing, GPS_BEARING)
    MAKE_NONEMPTY_INTEGRATOR(gpsSpeed, GPS_SPEED)
    MAKE_NONEMPTY_INTEGRATOR(magHeading, MAG_HEADING)
    MAKE_NONEMPTY_INTEGRATOR(watSpeed, WAT_SPEED)
#undef MAKE_NONEMPTY_INTEGRATOR

    int n = int(floor((upper - lower)/samplingPeriod));

    auto half = 0.5*samplingPeriod;

    ArrayBuilder<RawNav> samples(n);
    for (int i = 0; i < n; i++) {
      RawNav sample;

      auto from = lower + double(i)*samplingPeriod;
      auto to = from + samplingPeriod;

      sample.time = from + half;

      auto rawWatSpeed0 = watSpeed.computeAverage(from, to);
      if (rawWatSpeed0.defined()) {
        sample.watSpeed = rawWatSpeed0.get().value;
        auto rawMagHeading0 = magHeading.computeAverage(from, to);
        if (rawMagHeading0.defined()) {
          sample.magHeading = rawMagHeading0.get().value;
          auto rawGpsBearing0 = gpsBearing.computeAverage(from, to);
          if (rawGpsBearing0.defined()) {
            sample.gpsBearing = rawGpsBearing0.get().value;
            auto rawGpsSpeed0 = gpsSpeed.computeAverage(from, to);
            if (rawGpsSpeed0.defined()) {
              sample.gpsSpeed = rawGpsSpeed0.get().value;

              samples.add(sample);
            }
          }
        }
      }
    }
    return samples.get();
  }


  Array<Array<RawNav> > makeSampleGroups(
        const Array<NavDataset> &ds,
        const Duration<double> &samplingPeriod) {
    return sail::map(ds, [&](const NavDataset &ds) {
      return makeSamples(ds, samplingPeriod);
    });
  }

  MotionSamples applyCurrentCalibration(
      const BasicCorrectorParams<double> &params,
      const Array<RawNav> &samples) {
    int n = samples.size();
    BasicCurrentCorrector corr;
    MotionSamples dst(n);
    for (int i = 0; i < n; i++) {
      auto sample = samples[i];
      HorizontalMotion<double> v = corr.apply<double>(
          reinterpret_cast<const double *>(&params), sample)[0];

      dst[i] = TimedValue<HorizontalMotion<double> >(sample.time, v);
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

  auto params = CornerCalib::optimizeCornernessForGroups(BasicCurrentCorrector(),
      sampleGroups, cornerSettings);

  int n = sampleGroups.size();

  Array<MotionSamples> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = applyCurrentCalibration(params, sampleGroups[i]);
  }
  return dst;
}



}
}

