// Author: julien.pilet@gmail.com, 2014

#ifndef NAUTICAL_BOAT_MODEL_H
#define NAUTICAL_BOAT_MODEL_H

#include <cmath>
#include <server/common/logging.h>
#include <server/common/PhysicalQuantity.h>
#include <server/nautical/Nav.h>

namespace sail {

class BasicTrueWindEstimator {
  public:
    template <class T>
    static HorizontalMotion<T> computeTrueWind(const T* params, Array<Nav> past);

    template <class T>
    static void initializeParameters(T* params);

    enum {
        PARAM_AWA_OFFSET,
        PARAM_AWS_BIAS,
        NUM_PARAMS
    };
};

template <class T>
HorizontalMotion<T> BasicTrueWindEstimator::computeTrueWind(
        const T* params, Array<Nav> past) {
    // We could filter past measurements here.
    // However, for the sake of simplicity, we just take the last measurement.
    CHECK_LT(0, past.size());
    const Nav& measures = past.last();

    assert(!std::isnan(measures.gpsSpeed().metersPerSecond()));
    assert(!std::isnan(measures.gpsBearing().radians()));
    assert(!std::isnan(measures.awa().radians()));
    assert(!std::isnan(measures.aws().metersPerSecond()));

    HorizontalMotion<T> boatMotion = HorizontalMotion<T>::polar(
        measures.gpsSpeed().cast<T>(), measures.gpsBearing().cast<T>());

    // We assume no drift and no current.
    HorizontalMotion<T> appWindMotion = HorizontalMotion<T>::polar(
        measures.aws().cast<T>().scaled(params[PARAM_AWS_BIAS]),
        (measures.gpsBearing() + measures.awa()).cast<T>()
            + Angle<T>::degrees(params[PARAM_AWA_OFFSET]));

    // True wind - boat motion = apparent wind.
    return appWindMotion - boatMotion;
}

template<class T>
void BasicTrueWindEstimator::initializeParameters(T* params) {
    params[PARAM_AWA_OFFSET] = T(0);
    params[PARAM_AWS_BIAS] = T(1);
}

}  // namespace sail

#endif // NAUTICAL_BOAT_MODEL_H
