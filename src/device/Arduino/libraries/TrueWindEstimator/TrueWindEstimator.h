// Author: julien.pilet@gmail.com, 2014

#ifndef NAUTICAL_BOAT_MODEL_H
#define NAUTICAL_BOAT_MODEL_H

#ifdef ON_SERVER
#include <cmath>
#endif

#include "../PhysicalQuantity/PhysicalQuantity.h"
#include <algorithm>

namespace sail {

// When modifying this class, it might be necessary to duplicate it and to
// increase the MODEL_VERSION constant hereunder. Please read the
// MODEL_VERSION comment.
class TrueWindEstimator {
  public:
    // InstrumentAbstraction is typically an InstrumentFilter. It could also be
    // a Nav.
    template <class T, class InstrumentAbstraction>
    static HorizontalMotion<T> computeTrueWind(
        const T* params, const InstrumentAbstraction& measures);

    template <class T>
    static void initializeParameters(T* params);

    enum {
        PARAM_AWA_OFFSET,
        PARAM_UPWIND0,
        PARAM_DOWNWIND0,
        PARAM_DOWNWIND1,
        PARAM_DOWNWIND2,
        PARAM_DOWNWIND3,
        NUM_PARAMS,

        // Versionning is important. If the way parameters are used change,
        // this version number should be increased, and a new class should be
        // created, so that the server code can still generate calibration for
        // previous versions. Many versions can be saved in boat.dat.
        // The device will load only the version it has been compiled with.
        MODEL_VERSION=1,
    };

    template<typename T>
    struct Parameters {
      enum {
        STRUCT_IDENTIFIER=0x3000 + TrueWindEstimator::NUM_PARAMS,
        VERSION=0x1000 + TrueWindEstimator::MODEL_VERSION,
      };
      T params[TrueWindEstimator::NUM_PARAMS];
    };

};


template <typename T>
struct WindValues {
  HorizontalMotion<T> trueWind;
  T aws_bias;
  T awa_offset;
  T aws_offset;
  Angle<T> rawAwa;
  Velocity<T> rawAws;
};

template <typename T, typename InstrumentAbstraction>
WindValues<T> computeWindValues(
    const T* params, const InstrumentAbstraction& measures) {
  typedef typename InstrumentAbstraction::type WorkType;

  auto awa = measures.awa().normalizedAt0();
  WorkType awad = awa.degrees();

  bool upwind = (awad > WorkType(-90)) && (awad < WorkType(90));
  bool starboard = awad > WorkType(0);
  auto aws = measures.aws();

  // Due to a bug in NKE stuff and to an overflow in NmeaParser,
  // we sometime have negative values for aws that should be considered as 0.
  T awsk(std::max(0.0, double(aws.knots())));

  T awa_offset(params[TrueWindEstimator::PARAM_AWA_OFFSET]);
  T aws_bias(1.0);
  T aws_offset(0);

  T sideFactor(starboard ? 1 : -1);
  if (upwind) {
    awa_offset += sideFactor * ((awsk * awsk) * params[TrueWindEstimator::PARAM_UPWIND0]);
  } else {
    aws_offset = params[TrueWindEstimator::PARAM_DOWNWIND0];
    aws_bias += params[TrueWindEstimator::PARAM_DOWNWIND1] * awsk;
    awa_offset += sideFactor * ((awsk * awsk) * params[TrueWindEstimator::PARAM_DOWNWIND2]
                                +params[TrueWindEstimator::PARAM_DOWNWIND3]);
  }

  HorizontalMotion<T> boatMotion = measures.gpsMotion().template cast<T>();

  // We assume no drift and no current.
  HorizontalMotion<T> appWindMotion = HorizontalMotion<T>::polar(
      // For some reason yet to be investigated, the following line
      // produces a larger code than the line after.
      // Velocity<T>::knots(aws_offset) + static_cast<Velocity<T> >(measures.aws()).scaled(aws_bias),
      Velocity<T>::knots(aws_offset + awsk * aws_bias),
      static_cast<Angle<T> >(measures.gpsBearing() + awa + Angle<WorkType>::degrees(180))
          + Angle<T>::degrees(awa_offset));

  // True wind - boat motion = apparent wind.
  WindValues<T> dst;
  dst.trueWind = appWindMotion + boatMotion;
  dst.awa_offset = awa_offset;
  dst.aws_bias = aws_bias;
  dst.aws_offset = aws_offset;
  dst.rawAwa = awa;
  dst.rawAws = aws;
  return dst;
}


template <class T, class InstrumentAbstraction>
HorizontalMotion<T> TrueWindEstimator::computeTrueWind(
    const T* params, const InstrumentAbstraction& measures) {
    typedef typename InstrumentAbstraction::type WorkType;
  auto values = computeWindValues(params, measures);
  return values.trueWind;
}

template<class T>
void TrueWindEstimator::initializeParameters(T* params) {
  for (int i = 0; i < NUM_PARAMS; ++i) {
    params[i] = T(0);
  }
}

/*
 * Functions to compute TWA (which is NOT the same thing as TW.angle())
 *
 * Also, TW.angle() is the angle pointing in opposite
 * direction to the vector pointing at an angle TWDIR
 * */
template <typename T>
Angle<T> calcTwa(HorizontalMotion<T> calibratedTW,
    Angle<T> calibratedHeading) {
    HorizontalMotion<T> opposite(-calibratedTW[0], -calibratedTW[1]);
    return (opposite.angle() - calibratedHeading).positiveMinAngle();
}

template <typename T>
Angle<T> calcTwdir(HorizontalMotion<T> calibratedTW) {
    return (calibratedTW.angle()
        + Angle<T>::degrees(T(180))).positiveMinAngle();
}

template <typename T>
Velocity<T> calcTws(HorizontalMotion<T> calibratedTW) {
  return calibratedTW.norm();
}

template <typename T>
Velocity<T> calcVmg(Angle<> twa, Velocity<T> speed) {
  return cos(twa) * speed;
}

template <typename T>
HorizontalMotion<T> windMotionFromTwdirAndTws(Angle<T> twdir,
                                              Velocity<T> tws) {
  return HorizontalMotion<T>::polar(tws, twdir - Angle<>::degrees(180));
}

}  // namespace sail

#endif // NAUTICAL_BOAT_MODEL_H
