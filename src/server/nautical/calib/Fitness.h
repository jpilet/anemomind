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
#include <server/common/Span.h>
#include <server/math/BandedLevMar.h>

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

struct FullSettings {
  static const bool withBoatOverGround = true;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = true;
  static const bool withPitch = true;
  static const bool withIMU = true;
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

template <typename T>
struct Zero {
  static T get() {
    return MakeConstant<T>::apply(0.0);
  }
};

template <typename T>
struct Zero<Angle<T> > {
  static Angle<T> get() {
    return Angle<T>::radians(Zero<T>::get());
  }
};

template <typename T>
struct Zero<Velocity<T> > {
  static Velocity<T> get() {
    return Velocity<T>::knots(Zero<T>::get());
  }
};

template <typename T>
struct Zero<HorizontalMotion<T> > {
  static HorizontalMotion<T> get() {
    return HorizontalMotion<T>{
      Zero<Velocity<T>>::get(),
      Zero<Velocity<T>>::get()
    };
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

  void write(T **dst) const {
    if (variable) {
      TypeVectorizer<T, Type>::write(value, dst);
    }
  }

  Type value = Zero<Type>::get();
  Parameterized(const Type &x) : value(x) {}
};

// This object is used internally by the optimizer
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

  void read(const T **src) {
    boatOverGround.read(src);
    windOverGround.read(src);
    currentOverGround.read(src);
    heading.read(src);
    heel.read(src);
    pitch.read(src);
  }

  void write(T **dst) const {
    boatOverGround.write(dst);
    windOverGround.write(dst);
    currentOverGround.write(dst);
    heading.write(dst);
    heel.write(dst);
    pitch.write(dst);
  }

  // Suppose we have previously already reconstructed some of the state,
  // somehow. Then we can use that to initializer this object.
  static ReconstructedBoatState<T, Settings> make(const BoatState<T> &prototype) {
    ReconstructedBoatState<T, Settings> dst;

    // From the GPS filter...
    dst.boatOverGround = prototype.boatOverGround()
        .mapObjectValues([](double x) {
      return MakeConstant<T>::apply(x);
    });

    // Todo: Anything else to initialize?

    return dst;
  }
};

#define FOREACH_MEASURE_TO_CONSIDER(OP) \
  OP(AWA) \
  OP(AWS) \
  OP(MAG_HEADING) \
  OP(WAT_SPEED) \
  OP(ORIENT)

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
