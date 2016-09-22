/*
 * Fitness.h
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 *
 * This code is about generating the residuals for fitting to data.
 */

#ifndef SERVER_NAUTICAL_CALIB_FITNESS_H_
#define SERVER_NAUTICAL_CALIB_FITNESS_H_

#include <device/anemobox/Dispatcher.h>
#include <server/nautical/calib/SensorSet.h>
#include <server/nautical/BoatState.h>
#include <server/math/JetUtils.h>

namespace sail {

template <typename T>
struct BandWidthForType {};

template <>
struct BandWidthForType<Angle<double> > {
  static Angle<double> get() {
    return Angle<double>::degrees(5.0);
  }
};

template <>
struct BandWidthForType<Velocity<double> > {
  static Velocity<double> get() {
    return Velocity<double>::knots(0.5);
  }
};

// TODO: Study the literature on how noise is estimated
// in a Kalman filter, for different kinds of measurements.
// That is a related problem.
template <DataCode code>
struct BandWidth :
    BandWidthForType<typename TypeForCode<code>::type>{};

struct ServerBoatStateSettings {
  static const bool recoverGpsMotion = false;
  static const bool recoverHeelAngle = false;
  static const bool withIMU = false;
};

template <typename Settings>
struct BoatStateParamCount {
  static const int value = 2/*wind*/ + 2/*current*/ + 3/*orientation*/ +
      (Settings::recoverGpsMotion? 2 : 0);
};

template <typename T, typename DstType>
struct TypeVectorizer {};

template <typename T>
struct TypeVectorizer<T, HorizontalMotion<T> > {
  static HorizontalMotion<T> read(const T **src0) {
    const T *src = *src0;
    src0 += 2;
    return HorizontalMotion<T>(
        Velocity<T>::metersPerSecond(src[0]),
        Velocity<T>::metersPerSecond(src[1]));
  }

  static void write(const HorizontalMotion<T> &src, T **dst0) {
    T *dst = *dst0;
    dst0 += 2;
    dst[0] = src[0].metersPerSecond();
    dst[1] = src[1].metersPerSecond();
  }
};

template <typename T>
struct TypeVectorizer<T, Angle<T> > {
  static Angle<T> read(const T **src0) {
    const T *src = *src0;
    src0++;
    return Angle<T>::radians(src[0]);
  }

  static void write(Angle<T> src, T **dst0) {
    T *dst = *dst0;
    dst0++;
    dst[0] = src.radians();
  }
};


template <typename T, typename BoatStateSettings>
struct BoatStateVectorizer {
  BoatState<T> read(const BoatState<double> &base, const T *src0) {
    const T *src = src0;
    /*HorizontalMotion<T> wind = ReFromArray<T,
        HorizontalMotion<T> >::apply(&src);
    HorizontalMotion<T> current = ReadFromArray<T,
        HorizontalMotion<T> >::apply(&src);
*/
  }
};

template <DataCode code, typename BoatStateSettings>
class BoatStateFitness {
public:
  typedef typename TypeForCode<code>::type ObservationType;
  static const int inputCount =
      BoatStateParamCount<BoatStateSettings>::value;

  BoatStateFitness(
      double index,
      const ObservationType &value,
      BoatState<double> *base) :
    _realIndex(index), _observation(value),
    _base(base) {}

  template <typename T>
  bool evaluate(const T *X, T *y) const {
    BoatState<double> base = interpolate(
        _realIndex, _base[0], _base[1]);

    BoatState<T> a = BoatStateVectorizer<T,
        BoatStateSettings>::read(base, X + 0);
    BoatState<T> b = BoatStateVectorizer<T,
        BoatStateSettings>::read(base, X + inputCount);
    BoatState<T> x = interpolate(
        MakeConstant<T>::apply(_realIndex), a, b);
  }
private:
  double _realIndex;
  ObservationType _observation;
  BoatState<double> *_base;
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
