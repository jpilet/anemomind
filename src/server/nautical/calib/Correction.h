/*
 * Correction.h
 *
 *  Created on: Apr 29, 2016
 *      Author: jonas
 *
 * Two good papers on the topic of accurate measurements:
 *
 *
 */

#ifndef CALIB_CORRECTION_H_
#define CALIB_CORRECTION_H_

#include <math.h>
#include <server/common/Optional.h>
#include <server/nautical/common.h>
#include <server/nautical/AbsoluteOrientation.h>
#include <server/common/TimeStamp.h>

/*
 * I think we can distinguish three main cases of calibration:
 *
 * The case we would prefer:
 *  - We have measurements to obtain both wind over ground and current over ground
 *
 * Otherwise, it could be that for some boats:
 *  - We have measurements to obtain only an approximate wind estimate
 *  - We have measurements to obtain only an approximate current estimate
 *
 * So on the server, I guess we should prepare for these three scenarios, by
 * calibrating
 *  - A full set of parameters
 */

namespace sail {
namespace Correction {

/*
 * A sample of raw values
 *
 * Requirements:
 *   - All values must be finite
 *   - To represent potentially missing values, use Optional
 *
 * This sample should be ***independent*** of the underlying correction model,
 * so that we can use the same RawSample with an improved correction model.
 */
struct RawSample { // These are RAW values that we will correct

  sail::TimeStamp time;

  // Probably, we always require at least
  Angle<double> gpsBearing;
  Velocity<double> gpsSpeed;

  Optional<Velocity<double> > watSpeed;
  Optional<Angle<double> > magHeading;
  Optional<Angle<double> > awa;
  Optional<Velocity<double> > aws;

  Optional<AbsoluteOrientation> orientation;

  HorizontalMotion<double> gpsMotion() const {
    return HorizontalMotion<double>::polar(gpsSpeed, gpsBearing);
  }

  bool hasWindData() const {
    return awa.defined() && aws.defined();
  }

  bool hasCurrentData() const {
    return watSpeed.defined();
  }
};

namespace {
  double defaultDriftThresholdRadians = 0.5*M_PI;
}

template <typename T>
T upwindTwaDriftWeighting(T twaRadians_normalizedAt0,
    T thresholdRadians = T(defaultDriftThresholdRadians)) {

  // A fifth order polynomial with one root at 0 and dual roots
  // at +½pi and -½pi
  return twaRadians_normalizedAt0
    *sqr(twaRadians_normalizedAt0 - thresholdRadians)
    *sqr(twaRadians_normalizedAt0 + thresholdRadians);
}

template <typename T>
T twaDriftWeighting(Angle<T> twa0, T thresholdRadians
    = T(defaultDriftThresholdRadians)) {
  auto twa = twa0.normalizedAt0();
  auto twaRads = twa.radians();
  if (abs(twaRads) < T(thresholdRadians)) {
    return upwindTwaDriftWeighting(twaRads, thresholdRadians);
  }
  return T(0.0);
}

#define THIS_PARAM_DIM(TemplateClassName) \
  static const int dim = sizeof(TemplateClassName<T>)/sizeof(T);

// These are the corrector parameters
// We can imagine that, if we don't have a speedo, we may use these
// parameters with a corrector that only corrects the wind. In that case,
// the watSpeed-related parameters are unused.
#pragma pack(push, 1)
  template <typename T>
  struct BasicCorrectorParams {
    THIS_PARAM_DIM(BasicCorrectorParams)

    T awaOffset = T(0.0);        // angle
    T awsBias = T(1.0);          // dimensionless
    T awsOffset = T(0.0);        // velocity
    T magHeadingOffset = T(0.0); // angle
    T watSpeedOffset = T(0.0);   // velocity
    T watSpeedBias = T(1.0);     // dimensionless
    T driftCoefficient = T(0.0); // angle
  };
#pragma pop



template <typename T>
Angle<T> getCorrectedMagHeadingOrSubstitute(const RawSample &sample,
    const Angle<T> &offset) {
  return sample.magHeading.defined()?
          (sample.magHeading.get().cast<T>() + offset)
          : sample.gpsBearing.cast<T>();
}



// This class describes how the correction parameters are applied to a sample
// in order to get wind and current.
struct BasicFullCorrector { // Using SI units: angles in radians, velocities in m/s

  // Used by the optimizer to provide an initialization
  typedef BasicCorrectorParams<double> InitialParamType;

  static const int FlowCount = 2;

  // Expected that this method is present by the calibration algorithm
  HorizontalMotion<double> getRefMotion(const RawSample &sample) const {
    return sample.gpsMotion();
  }

  // Called by the optimization algorithm
  template <typename T>
  std::array<HorizontalMotion<T>, FlowCount> apply(
      const T *params0, const RawSample &sample) const {
    const auto &p = *(reinterpret_cast<const BasicCorrectorParams<T> *>(params0));

    // We can only perform full calibration if these conditions are satisfied.
    assert(sample.hasCurrentData());
    assert(sample.hasWindData());

    auto rad = Angle<T>::radians(T(1.0));
    auto mps = Velocity<T>::metersPerSecond(T(1.0));

    // Do our best to estimate the heading. If no magnetic heading is present,
    // maybe we can use GPS bearing as an approximation? Or would it be better
    // to refuse to correct?
    auto heading = getCorrectedMagHeadingOrSubstitute<T>(sample, p.magHeadingOffset*rad);

    auto awVectorAngle = heading
        + sample.awa.get().cast<T>()
        + (p.awaOffset + T(M_PI))*rad;

    auto awVectorNorm = p.awsBias*sample.aws.get().cast<T>() + p.awsOffset*mps;

    auto AW = HorizontalMotion<T>::polar(awVectorNorm, awVectorAngle);

    auto boatMotion = sample.gpsMotion().cast<T>();

    auto windOverGround = computeWindOverGroundFromApparentWindAndBoatMotion(
        AW, boatMotion);

    auto twdir = computeTwdirFromWindOverGround(windOverGround);

    auto twa = computeTwaFromTwdirAndHeading(twdir, heading).normalizedAt0();

    auto driftWeighting = twaDriftWeighting<T>(twa);

    auto boatMotionOverWaterAngle = p.driftCoefficient*driftWeighting*rad + heading;

    auto boatMotionOverWaterNorm = p.watSpeedBias*sample.watSpeed.get().cast<T>()
        + p.watSpeedOffset*mps;

    auto boatMotionOverWater = HorizontalMotion<double>::polar(
        boatMotionOverWaterNorm,
        boatMotionOverWaterAngle);

    auto current = computeCurrentFromBoatMotionOverWaterAndGround(
        boatMotionOverWater, boatMotion);

    return std::array<HorizontalMotion<T>, FlowCount>{
      windOverGround, current
    };
  }
};

// In case wind information is missing, but water information is present
struct BasicCurrentCorrector {
  typedef BasicCorrectorParams<double> InitialParamType;
  static const int FlowCount = 1;

  HorizontalMotion<double> getRefMotion(const RawSample &sample) const {
    return sample.gpsMotion();
  }

  template <typename T>
  std::array<HorizontalMotion<T>, FlowCount> apply(
      const T *params0, const RawSample &sample) const {
    const auto &p = *(reinterpret_cast<const BasicCorrectorParams<T> *>(params0));
    assert(sample.hasCurrentData());

    auto rad = Angle<T>::radians(T(1.0));
    auto mps = Velocity<T>::metersPerSecond(T(1.0));

    // Do our best to estimate the heading. If no magnetic heading is present,
    // maybe we can use GPS bearing as an approximation? Or would it be better
    // to refuse to correct?
    auto heading = getCorrectedMagHeadingOrSubstitute<T>(sample, p.magHeadingOffset*rad);

    auto boatMotion = sample.gpsMotion().cast<T>();

    auto boatMotionOverWaterAngle = heading;

    auto boatMotionOverWaterNorm = p.watSpeedBias*sample.watSpeed.get().cast<T>()
        + p.watSpeedOffset*mps;

    auto boatMotionOverWater = HorizontalMotion<T>::polar(
        boatMotionOverWaterNorm,
        boatMotionOverWaterAngle);

    auto current = computeCurrentFromBoatMotionOverWaterAndGround(
        boatMotionOverWater, boatMotion);

    return std::array<HorizontalMotion<T>, FlowCount>{current};
  }
};

}
}

#endif
