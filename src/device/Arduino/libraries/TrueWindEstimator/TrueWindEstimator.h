// Author: julien.pilet@gmail.com, 2014

#ifndef NAUTICAL_BOAT_MODEL_H
#define NAUTICAL_BOAT_MODEL_H

#include <cmath>
#include "../PhysicalQuantity/PhysicalQuantity.h"

namespace sail {


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
        NUM_PARAMS
    };
};

template <class T, class InstrumentAbstraction>
HorizontalMotion<T> TrueWindEstimator::computeTrueWind(
        const T* params, const InstrumentAbstraction& measures) {
    typedef typename InstrumentAbstraction::type WorkType;
    WorkType awa = measures.awa().normalizedAt0().degrees();

    bool upwind = (awa > WorkType(-90)) && (awa < WorkType(90));
    bool starboard = awa > WorkType(0);
    T aws(measures.aws().knots()); 

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

    HorizontalMotion<T> boatMotion = HorizontalMotion<T>::polar(
        static_cast<Velocity<T> >(measures.gpsSpeed()),
        static_cast<Angle<T> >(measures.gpsBearing()));

    // We assume no drift and no current.
    HorizontalMotion<T> appWindMotion = HorizontalMotion<T>::polar(
        Velocity<T>::knots(aws_offset) + static_cast<Velocity<T> >(measures.aws()).scaled(aws_bias),
        static_cast<Angle<T> >(measures.gpsBearing() + measures.awa())
            + Angle<T>::degrees(awa_offset));

    // True wind - boat motion = apparent wind.
    return appWindMotion - boatMotion;
}

template<class T>
void TrueWindEstimator::initializeParameters(T* params) {
  for (int i = 0; i < NUM_PARAMS; ++i) {
    params[i] = T(0);
  }
}

}  // namespace sail

#endif // NAUTICAL_BOAT_MODEL_H
