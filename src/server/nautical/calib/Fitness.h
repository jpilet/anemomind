/*
 * Fitness.h
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 *
 * This code is about generating the residuals for fitting to data.
 *
 * Interesting papers:
 *
 * - "HEEL SAILING YACHT PERFORMANCE THE EFFECTS OF ANGLE AND LEEWAY ANGLE ON RESISTANCE AND SIDEFORCE"
 *   http://160.75.46.2/staff/insel/Publications/Cesme.PDF
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

template <typename T, typename system, int TimeDim/*t*/,
  int LengthDim/*l*/, int AngleDim/*a*/, int MassDim/*m*/>
struct MakeConstant<PhysicalQuantity<T, system, TimeDim,
LengthDim, AngleDim, MassDim>> {
  static PhysicalQuantity<T, system, TimeDim,
      LengthDim, AngleDim, MassDim> apply(
          PhysicalQuantity<double, system, TimeDim,
            LengthDim, AngleDim, MassDim> x) {
    return x.mapObjectValues([](double x) {
      return MakeConstant<T>::apply(x);
    });
  }
};

template <typename T>
T sqrtHuber(T x) {
  static const T zero = MakeConstant<T>::apply(0.0);
  static const T one = MakeConstant<T>::apply(1.0);
  static const T two = MakeConstant<T>::apply(2.0);
  if (x < zero) {
    return -sqrtHuber(-x);
  } else {
    return x < one? x : sqrt(one + two*(x - one));
  }
}

template <typename T>
struct DefaultUndefinedResidual {
  static T get() {return MakeConstant<T>::apply(4.0);}
};

template <typename T, typename Q>
struct BandWidthForType {};

template <typename T>
struct BandWidthForType<T, Angle<double> > {
  static Angle<T> get() {
    return Angle<T>::degrees(MakeConstant<double>::apply(5.0));
  }
};

template <typename T>
struct BandWidthForType<T, Velocity<double> > {
  static Velocity<T> get() {
    return Velocity<T>::knots(MakeConstant<double>::apply(0.5));
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

template <typename T>
Velocity<T> referenceVelocityForAngles() {
  return referenceVelocityForAngles<T>(
      BandWidthForType<T, Velocity<double>>::get(),
      BandWidthForType<T, Angle<double>>::get());
}

// TODO: Study the literature on how noise is estimated
// in a Kalman filter, for different kinds of measurements.
// That is a related problem.
/*
 * http://jbrwww.che.wisc.edu/tech-reports/twmcc-2003-04.pdf
 */

template <typename T, DataCode code>
struct BandWidth :
    BandWidthForType<T, typename TypeForCode<code>::type>{};

template <typename T>
struct BandWidth<T, AWA> :
  public BandWidthForType<T, Velocity<double>> {};

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

  Optional<AbsoluteBoatOrientation<T>> orientation() const {
    auto h = heading.value.optionalAngle();
    if (h.defined()) {
      return AbsoluteBoatOrientation<T>{
        heading, heel, pitch
      };
    }
    return Optional<AbsoluteBoatOrientation<T>>();
  }
};

template <typename T, typename Settings>
struct AWAFitness {
  static const int outputCount = 1;

  static bool apply(
      const ReconstructedBoatState<T, Settings> &state,
      const DistortionModel<T, AWA> &distortion,
      const Angle<double> &observation,
      T *residuals) {
    residuals[0] = DefaultUndefinedResidual<T>::get();
    auto h = state.heading.value.optionalAngle();
    if (h.defined()) {
      HorizontalMotion<T> cleanAW = computeApparentWind<T>(
          state.boatOverGround.value,
          state.windOverGround.value);
      auto distortedAW = distortion.apply(cleanAW);
      auto observedAW = makeApparentWind(
          cleanAW.norm(),
          MakeConstant<Angle<T>>::apply(observation),
          h.get());
      auto error = HorizontalMotion<T>(distortedAW - observedAW);
      auto bw = BandWidth<T, AWA>::get();
      residuals[0] = sqrtHuber<T>(T(error.norm()/bw));
    }
    return true;
  }
};

template <typename T, typename Settings>
struct AWSFitness {
  static const int outputCount = 1;

  static bool apply(const ReconstructedBoatState<T, Settings> &state,
      const DistortionModel<T, AWS> &distortion,
      const Velocity<double> &observation,
      T *residuals) {
    auto aw = computeApparentWind<double>(
        state.boatOverGround.value, state.windOverGround.value);
    auto bw = BandWidth<T, AWS>::get();
    Velocity<T> error = distortion.apply(aw.norm())
        - MakeConstant<Velocity<T>>::apply(observation);
    residuals[0] = sqrtHuber<T>(error/bw);
    return true;
  }
};

template <typename T, typename Settings>
struct MagHeadingFitness {
  static const int outputCount = 1;

  static bool apply(const ReconstructedBoatState<T, Settings> &state,
                    const DistortionModel<T, MAG_HEADING> &distortion,
                    const Angle<double> &observation,
                    T *residuals) {
    auto bw = BandWidth<T, MAG_HEADING>::get();
    auto velBW = BandWidthForType<T, Velocity<double>>::get();
    auto observedHeadingVector = HorizontalMotion<T>::polar(
        referenceVelocityForAngles<T>(velBW, bw),
        MakeConstant<Angle<T>>::apply(observation));
    auto distortedHeadingVector = distortion.apply(state.heading.value);
    auto error = HorizontalMotion<T>(
        observedHeadingVector - distortedHeadingVector).norm();
    residuals[0] = sqrtHuber<T>(error/velBW);
    return true;
  }
};

template <typename T, typename Settings>
struct WatSpeedFitness {
  static const int outputCount = 1;

  static bool apply(const ReconstructedBoatState<T, Settings> &state,
                    const DistortionModel<T, WAT_SPEED> &distortion,
                    const Velocity<double> &observation,
                    T *residuals) {
    auto boatOverWater = computeBoatOverWater<T>(
        state.boatOverGround.value,
        state.currentOverGround.value);
    auto error = distortion.apply(boatOverWater.norm()) -
        MakeConstant<Velocity<T>>::apply(observation);
    auto bw = BandWidth<T, WAT_SPEED>::get();
    residuals[0] = sqrtHuber<T>(error/bw);
    return true;
  }
};

// Compare the boat-to-world rotation matrices
// (the one of the state and the observed one)
template <typename T, typename Settings>
struct OrientFitness {
  static const int outputCount = 1;
  static bool apply(const ReconstructedBoatState<T, Settings> &state,
                    const DistortionModel<T, ORIENT> &distortion,
                    const AbsoluteOrientation &observation,
                    T *residuals) {
    auto orient = state.orientation();
    if (orient.defined()) {
      auto obsR = BNO055AnglesToRotation(observation);
      //auto boatR =
    }
    return true;
  }

};

// Not related to any particular sensor
template <typename T, typename Settings>
struct HeelFitness {

};

// Not related to any particular sensor
template <typename T, typename Settings>
struct DriftFitness {

};

#define FOREACH_MEASURE_TO_CONSIDER(OP) \
  OP(AWA) \
  OP(AWS) \
  OP(MAG_HEADING) \
  OP(WAT_SPEED) \
  OP(ORIENT)

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
