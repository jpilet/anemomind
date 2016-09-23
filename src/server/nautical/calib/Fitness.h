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

// Used to reinterpret angles as velocity vectors.
template <typename T>
Velocity<T> referenceVelocityForAngles(
    Velocity<T> velocityWidth,
    Angle<T> angleWidth) {
  /*
   * Reasoning:
   *
   * An angle in radians multiplied by a radius gives an arc length.
   * The arc length in this case is velocityWidth. The radius is unknown.
   * The angle is angleWidth. So from that we get the expression below.
   */
  return (MakeConstant<T>::apply(1.0)/angleWidth.radians())*velocityWidth;
}

// TODO: Study the literature on how noise is estimated
// in a Kalman filter, for different kinds of measurements.
// That is a related problem.
/*
 * http://jbrwww.che.wisc.edu/tech-reports/twmcc-2003-04.pdf
 */
template <DataCode code>
struct BandWidth :
    BandWidthForType<typename TypeForCode<code>::type>{};

struct ServerBoatStateSettings {
  static const bool recoverGpsMotion = false;
  static const bool recoverHeelAngle = false;
  static const bool recoverPitch = false;
  static const bool withIMU = false;
};

template <typename Settings>
struct BoatStateParamCount {
  static const int value = 2/*wind*/ + 2/*current*/
      + 1 /*heading*/
      + (Settings::recoverHeelAngle? 1 : 0)
      + (Settings::recoverPitch? 1 : 0)
      + (Settings::recoverGpsMotion? 2 : 0);
};

// So that we can read a numeric representation
// of a type easily.
template <typename T, typename Type>
struct TypeVectorizer {
  static Type read(const T **src0) {
    Type dummy;
    return dummy.mapObjectValues([&](T) {
      T x = **src0;
      (*src0)++;
      return x;
    });
  }

  static void write(const Type &src, T **dst0) {
    src.mapObjectValues([&](T x) {
      (**dst0) = x;
      (*dst0)++;
      return x; // not used.
    });
  }
};



template <typename T, typename BoatStateSettings>
struct BoatStateVectorizer {
  BoatState<T> read(const BoatState<double> &base, const T *src0) {
    const T *src = src0;

    //BoatState<T> baseT = initializeBoatState<T>(base);

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
