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

template <DataCode code>
struct BandWidth :
    BandWidthForType<typename TypeForCode<code>::type>{};

struct ServerBoatStateSettings {
  static const bool recoverGpsMotion = false;
};

struct DeviceBoatStateSettings {
  static const bool recoverGpsMotion = true;
};

template <typename Settings>
struct BoatStateParamCount {
  static const int value = 2/*wind*/ + 2/*current*/ + 3/*orientation*/ +
      (Settings::recoverGpsMotion? 2 : 0);
};

template <typename T, typename BoatStateSettings>
struct BoatStateVectorizer {
  BoatState<T> read(const T *src) {

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
      const Array<BoatState<double> > &base) :
    _realIndex(index), _observation(value) {}

  template <typename T>
  bool evaluate(const T *X, T *y) const {
    BoatState<double> base = interpolate(
        _realIndex, _base[0], _base[1]);

    BoatState<T> a = BoatStateVectorizer<T,
        BoatStateSettings>::read(X + 0);
    BoatState<T> b = BoatStateVectorizer<T,
        BoatStateSettings>::read(X + inputCount);
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
