// Author: julien.pilet@gmail.com, 2014

#ifndef NAUTICAL_BOAT_MODEL_H
#define NAUTICAL_BOAT_MODEL_H

#include <cmath>
#include <server/common/PhysicalQuantity.h>
#include <server/common/logging.h>
#include <server/nautical/Nav.h>

namespace sail {

namespace {

Nav averageNavs(Array<Nav> past, Duration<> duration) {
  CHECK_LT(0, past.size());

  HorizontalMotion<double> boatMotion(
      Velocity<double>::metersPerSecond(0),
      Velocity<double>::metersPerSecond(0));

  HorizontalMotion<double> apparentWind(
      Velocity<double>::metersPerSecond(0),
      Velocity<double>::metersPerSecond(0));

  HorizontalMotion<double> waterMotion(boatMotion);

  int n = 0;
  for (int i = past.size() - 1; i >= 0; --i) {
    const Nav& entry = past[i];
    if ((past.last().time() - entry.time()) < duration) {
      boatMotion = boatMotion + HorizontalMotion<double>::polar(
          entry.gpsSpeed(), entry.gpsBearing());

      apparentWind = apparentWind + HorizontalMotion<double>::polar(
          entry.aws(), entry.awa());

      waterMotion = waterMotion +  HorizontalMotion<double>::polar(
          entry.watSpeed(), entry.magHdg());
      ++n;
    }
  }

  boatMotion = boatMotion.scaled(1.0 / n);
  apparentWind = apparentWind.scaled(1.0 / n);
  waterMotion = waterMotion.scaled(1.0 / n);

  Nav result;
  result.setAwa(apparentWind.angle());
  result.setAws(apparentWind.norm());
  result.setGpsSpeed(boatMotion.norm());
  result.setGpsBearing(boatMotion.angle());
  result.setMagHdg(waterMotion.angle());
  result.setWatSpeed(waterMotion.norm());
  return result;
}

}  // namespace

class BasicTrueWindEstimator {
  public:
    template <class T>
    static void computeAppWindValues(const T *params, const Nav &nav,
        Velocity<T> *outAws, Angle<T> *outAwa);

    template <class T>
    static HorizontalMotion<T> computeAppWindMotion(const T *params, const Nav &nav);

    template <class T>
    static HorizontalMotion<T> computeTrueWind(const T* params, Array<Nav> past);

    template <class T>
    static HorizontalMotion<T> computeTrueWind(const T* params, const Nav &nav);

    template <class T>
    static void initializeParameters(T* params);

    enum {
        PARAM_AWA_OFFSET,
        PARAM_UPWIND0,
        PARAM_DOWNWIND0,
        PARAM_DOWNWIND1,
        PARAM_DOWNWIND2,
        PARAM_DOWNWIND3,
        NUM_PARAMS
    };

    // "Raw" in the sense that it assumes that the compass doesn't need to be calibrated.
    template <class T>
    static HorizontalMotion<T> computeRawBoatMotion(const Nav &past);

  private:
};

template <class T>
void BasicTrueWindEstimator::computeAppWindValues(const T *params, const Nav &nav,
  Velocity<T> *outAws, Angle<T> *outAwa) {
  assert(!std::isnan(nav.gpsSpeed().metersPerSecond()));
  assert(!std::isnan(nav.gpsBearing().radians()));
  assert(!std::isnan(nav.awa().radians()));
  assert(!std::isnan(nav.aws().metersPerSecond()));

  double awa = normalizeAngleBetweenMinusPiAndPi(nav.awa().radians());

  bool upwind = (awa > (- M_PI / 2.0)) && (awa < (M_PI / 2.0));
  bool starboard = awa > 0;
  T aws(nav.aws().knots());

  T awa_offset(params[PARAM_AWA_OFFSET]);
  T aws_bias(1.0);
  T aws_offset(0);

  T sideFactor(starboard ? 1 : -1);
  if (upwind) {
    awa_offset += sideFactor * ((aws * aws) * params[PARAM_UPWIND0]);
  } else {
    aws_offset = params[PARAM_DOWNWIND0];
    aws_bias += params[PARAM_DOWNWIND1] * aws;
    awa_offset += sideFactor * ((aws * aws) * params[PARAM_DOWNWIND2]
                                +params[PARAM_DOWNWIND3]);
  }

  *outAws = Velocity<T>::knots(aws_offset) + nav.aws().cast<T>().scaled(aws_bias);
  *outAwa = nav.awa().cast<T>() + Angle<T>::degrees(awa_offset);
}

template <class T>
HorizontalMotion<T> BasicTrueWindEstimator::computeAppWindMotion(const T *params, const Nav &nav) {
  Velocity<T> aws;
  Angle<T> awa;
  computeAppWindValues(params, nav, &aws, &awa);
  return HorizontalMotion<T>::polar(aws,
      awa + nav.gpsBearing().cast<T>());
}




template <class T>
HorizontalMotion<T> BasicTrueWindEstimator::computeTrueWind(
        const T* params, Array<Nav> past) {
    CHECK(!past.empty());
    return computeTrueWind(params,
        averageNavs(past, Duration<>::seconds(20)));
}

template <class T>
HorizontalMotion<T> BasicTrueWindEstimator::computeTrueWind(
        const T* params, const Nav &nav) {
    // True wind - boat motion = apparent wind.
    return computeAppWindMotion<T>(params, nav) - computeRawBoatMotion<T>(nav);
}


template <class T>
HorizontalMotion<T> BasicTrueWindEstimator::computeRawBoatMotion(const Nav &past) {
  return HorizontalMotion<T>::polar(
      past.gpsSpeed().cast<T>(), past.gpsBearing().cast<T>());

}

template<class T>
void BasicTrueWindEstimator::initializeParameters(T* params) {
  for (int i = 0; i < NUM_PARAMS; ++i) {
    params[i] = T(0);
  }
}

}  // namespace sail

#endif // NAUTICAL_BOAT_MODEL_H
