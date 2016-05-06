/*
 * Correction.h
 *
 *  Created on: Apr 29, 2016
 *      Author: jonas
 *
 * Two good papers on the topic of accurate measurements:
 *
 * (i)
@inproceedings{douguet:hal-00846250,
  TITLE = {{A New Real-Time Method for Sailboat Performance estimation based on Leeway Modeling}},
  AUTHOR = {Douguet, Ronan and Diguet, Jean-Philippe and Laurent, Johann and Riou, Yann},
  URL = {https://hal.archives-ouvertes.fr/hal-00846250},
  BOOKTITLE = {{The 21st Chesapeake Sailing Yacht Symposium}},
  ADDRESS = {Annapolis, United States},
  PAGES = {A New Real-Time Method for Sailboat Performance estimation based on Leeway Modeling},
  YEAR = {2013},
  MONTH = Mar,
  HAL_ID = {hal-00846250},
  HAL_VERSION = {v1},
}
 *
 *
 * (ii)
 * "Yacht Performance Analysis with Computers"
 * THE SOCIETY OF NAVAL ARCHITECTS AND MARINE ENGINEERS
 * One World Trade Center, Suite 1369, New York, N.Y. 10048
 * Paper to be presented at the Chesapeake Sailing Yacht Symposium, Annapolis, Maryland, January 17, 1981.
 * David R. Pedrick, Pedrick Yacht Designs, Newport, Rl.
 * RichardS. McCurdy, Consultant, Darien, CT.
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
struct RawNav { // These are RAW values that we will correct

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
  Angle<double> defaultDriftThreshold = Angle<double>::radians(0.5*M_PI);
}


template <typename T>
T upwindTwaDriftWeighting(
    T twa_at0,
    T threshold) {

  // A fifth order polynomial with one root at 0 and dual roots
  // at +½pi and -½pi
  return twa_at0
    *sqr(twa_at0 - threshold)
    *sqr(twa_at0 + threshold);
}

template <typename T>
T twaDriftWeighting(Angle<T> twa0, Angle<T> threshold
    = defaultDriftThreshold.cast<T>()) {
  auto twa = twa0.normalizedAt0();
  if (fabs(twa) < threshold) {
    return upwindTwaDriftWeighting(twa.radians(), threshold.radians());
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
Angle<T> getCorrectedMagHeadingOrSubstitute(const RawNav &sample,
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
  HorizontalMotion<double> getRefMotion(const RawNav &sample) const {
    return sample.gpsMotion();
  }

  // Called by the optimization algorithm
  template <typename T>
  std::array<HorizontalMotion<T>, FlowCount> apply(
      const T *params0, const RawNav &sample) const {
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

  HorizontalMotion<double> getRefMotion(const RawNav &sample) const {
    return sample.gpsMotion();
  }

  template <typename T>
  std::array<HorizontalMotion<T>, FlowCount> apply(
      const T *params0, const RawNav &sample) const {
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
