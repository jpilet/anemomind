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
  static const bool withBoatOverGround = false;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = false;
  static const bool withPitch = false;
  static const bool withIMU = false;
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

// This type is used to control whether a value should
// have corresponding optimization parameters or kept
// constant. That choice is made at compile-time.
template <typename Type, typename T, bool variable>
struct Parameterized {
  static const int valueDimension = variable? Type::valueDimension : 0;
  Parameterized() {}

  template <typename ProtoType>
  Parameterized make(const ProtoType &prototype) {
    return Parameterized(prototype.mapObjectValues([](double x) {
      return MakeConstant<T>::apply(x);
    }));
  }

  void read(const T **src) {
    if (variable) {
      value = TypeVectorizer<T, Type>::read(src);
    }
  }

  Type value;
  Parameterized(const Type &x) : value(x) {}
};

template <typename T, typename Settings>
struct ReconstructedBoatState {
  typedef Parameterized<HorizontalMotion<T>,
      T, Settings::withBoatOverGround> BOG;
  BOG boatOverGround;

  typedef Parameterized<HorizontalMotion<T>,
      T, true> WOG;
  WOG windOverGround;

  typedef Parameterized<HorizontalMotion<T>,
      T, Settings::withCurrentOverGround> COG;
  COG currentOverGround;

  // Heading is represented as a horizontal motion
  // to help optimizer convergence
  typedef Parameterized<HorizontalMotion<T>,
      T, true> Heading;
  Heading heading;

  typedef Parameterized<Angle<T>, T, Settings::withHeel> Heel;
  Heel heel;

  typedef Parameterized<Angle<T>, T, Settings::withPitch> Pitch;
  Pitch pitch;

  static const int valueDimension =
        BOG::valueDimension + WOG::valueDimension + COG::valueDimension
        + Heading::valueDimension + Heel::valueDimension
        + Pitch::valueDimension;
};


template <DataCode code, typename BoatStateSettings>
class BoatStateFitness {
public:
  typedef typename TypeForCode<code>::type ObservationType;
  /*static const int inputCount =
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
  }*/
private:
  double _realIndex;
  ObservationType _observation;
  BoatState<double> *_base;
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
