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

#include <server/nautical/BoatState.h>
#include <server/math/JetUtils.h>
#include <server/common/Span.h>
#include <server/nautical/calib/BoatParameters.h>

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
   * Explanation:
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

template <typename T>
struct BandWidth<T, ORIENT> :
  public BandWidthForType<T, Angle<double>> {};


template <typename T, typename nonsense = bool>
struct AreFitnessSettings {
  static const bool value = false;
};
template <typename T>
struct AreFitnessSettings<T,
  decltype(
    T::withBoatOverGround ||
    T::withWindOverGround ||
    T::withCurrentOverGround ||
    T::withHeel ||
    T::withPitch)> {
  static const bool value = true;
};

// The settings specify what we want to reconstruct
struct DefaultSettings {
  // Provided by the GPS filter
  static const bool withBoatOverGround = false;

  static const bool withWindOverGround = true;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = true;
  static const bool withPitch = true;
};

// Future version, also boat motion recover.
struct FullSettings {
  static const bool withBoatOverGround = true;
  static const bool withWindOverGround = true;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = true;
  static const bool withPitch = true;
};

static_assert(AreFitnessSettings<FullSettings>::value, "Bad settings");


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
      // To avoid undefined angles. Maybe a random number would be better?
      Velocity<T>::knots(MakeConstant<T>::apply(1.0e-9)),
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

  void readAndStepPtr(const T **src) {
    if (variable) {
      value = TypeVectorizer<T, Type>::read(src);
    }
  }

  void writeAndStepPtr(T **dst) const {
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
  // to help optimizer convergence.
  typedef Parameterized<HorizontalMotion<T>,
      T, true> Heading;
  Heading heading;

  typedef Parameterized<Angle<T>, T, Settings::withHeel> Heel;
  Heel heel;

  typedef Parameterized<Angle<T>, T, Settings::withPitch> Pitch;
  Pitch pitch;

  static const int dynamicValueDimension =
        BOG::valueDimension + WOG::valueDimension + COG::valueDimension
        + Heading::valueDimension + Heel::valueDimension
        + Pitch::valueDimension;

  void readFrom(const T *src) {
    boatOverGround.readAndStepPtr(&src);
    windOverGround.readAndStepPtr(&src);
    currentOverGround.readAndStepPtr(&src);
    heading.readAndStepPtr(&src);
    heel.readAndStepPtr(&src);
    pitch.readAndStepPtr(&src);
  }

  void writeTo(T *dst) const {
    boatOverGround.writeAndStepPtr(&dst);
    windOverGround.writeAndStepPtr(&dst);
    currentOverGround.writeAndStepPtr(&dst);
    heading.writeAndStepPtr(&dst);
    heel.writeAndStepPtr(&dst);
    pitch.writeAndStepPtr(&dst);
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
        heading.value.angle(),
            heel.value, pitch.value
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

      // We are going to measure the angle that relates one
      // boat-to-world rotation matrix (from the sensor) to the
      // other boat-to-world roation matrix (estimated from the state).

      auto obsR = distortion.computeBoatToWorldRotation(
          BNO055AnglesToRotation(observation));
      auto boatR = orientationToMatrix<T>(orient.get());
      // obsR = relR*boatR <=> relR = obsR*boatR'
      auto relR = obsR*boatR.transpose();

      // According to https://en.wikipedia.org/wiki/Rotation_matrix#Determining_the_angle
      // we can compute cos(theta) from the trace (theta being the angle):
      Velocity<T> hNorm = state.heading.value.norm();
      T cosTheta = (relR.trace()
          - MakeConstant<T>::apply(1.0))/MakeConstant<T>::apply(2.0);

      // Because the heading is represented using a vector,
      // we are going to formulate the residual so that
      // the length of that vector approaches a reference
      // velocity, in addition to optimizing all the angles.
      auto bw = BandWidth<T, ORIENT>::get();
      auto velbw = BandWidthForType<T, Velocity<double>>::get();
      auto refVel = referenceVelocityForAngles(velbw, bw);
      T squaredXError = sqr<T>((hNorm*cosTheta - refVel)/velbw);
      T squaredYError = sqr<T>(hNorm/velbw)*(
          MakeConstant<T>::apply(1.0) - cosTheta*cosTheta);
      T totalError = sqrt(MakeConstant<T>::apply(1.0e-9)
          + squaredXError + squaredYError);
      residuals[0] = sqrtHuber<T>(totalError);
    }
    return true;
  }
};

    /*PhysicalQuantity<T,
    UnitSystem::CustomAnemoUnits,
    1, -1, 1, 0>;*/

// Not related to any particular sensor
// Here we suppose that the heel angle is roughly proportional to the
// apparent wind speed projected on the X basis vector of the boat
// (see BoatState.h for a drawing). I am not sure how true this generally
// is. Maybe we should increase the band width a little bit to accomodate
// for the uncertainty...
//
// TODO: If we don't attempt to recover the heel angle,
// we should (withHeel = false), then heelPerWindSpeed
// should be overridden by 0 (here or elsewhere).
template <typename T, typename Settings>
struct HeelFitness {
  static const int outputCount = 1;

  static Angle<T> bandWidth() {
    return MakeConstant<T>::apply(4.0)
        *BandWidthForType<T, Angle<double>>::get();
  }

  static bool apply(const ReconstructedBoatState<T, Settings> &state,
                    const HeelConstant<T> &heelConstant,
                    T *residuals) {
    residuals[0] = DefaultUndefinedResidual<T>::get();
    auto hnorm = state.heading.value.norm();
    if (MakeConstant<T>::apply(0.0) < hnorm.knots()) {
      T x = state.heading.value[1]/hnorm;
      T y = -state.heading.value[0]/hnorm;
      auto aw = computeApparentWind<T>(
          state.boatOverGround.value,
          state.windOverGround.value);
      Velocity<T> proj = x*aw[0] + y*aw[1];
      Angle<T> error = state.heel.value - heelConstant*proj;
      residuals[0] = sqrtHuber<T>(error/bandWidth());
    }
    return true;
  }
};

// Not related to any particular sensor
// TODO: If we don't attempt to model leeway, the heading of
// the boat should coincide with the angle of the vector
// of the boatOverWater (which is a maxLeewayAngle of 0)
//
// If we have heel angle estimated, let's use the simple expression
// under section "Leeway Algorithm":
//
//  lambda: Leeway angle
//  k: Overall proportionality constant
//  phi: Heel angle
//  v_s: Boat speed (over water, I assume)
//
//  Then we predict Leeway angle as
//
//  lambda = k*phi/(v_s^2)
//
//  Here is the paper:
//     Yacht Performance Analysis with Computers
//     David R. Pedrick, Pedrick Yacht Designs, Newport, Rl. RichardS. McCurdy, Consultant, Darien, CT.
//
//     FOUND HERE: http://www.sname.org/HigherLogic/System/DownloadDocumentFile.ashx?DocumentFileKey=5d932796-f926-4262-88f4-aaca17789bb0
//
// Also, Fig. 2 in that paper is useful.
// This entire paper contains interesting corrections that we can use!!!

// I am not exactly sure about the concepts here.
// Is "leeway" angle the right word for this. It seems that
// if an object drifs sligtly downwind, then the leeway angle would
// be positive according to this Wikipedia page:
// https://en.wikipedia.org/wiki/Leeway
// But for our treatment, it is more convenient to treat it just as
// the difference between the heading and the boat course over water.
template <typename T, typename Settings>
struct LeewayFitness {
  static const int outputCount = 1;

  static bool apply(const ReconstructedBoatState<T, Settings> &state,
      const LeewayConstant<T> k,
      T *residuals) {

    residuals[0] = DefaultUndefinedResidual<T>::get();
    auto heading0 = state.heading.value.optionalAngle();
    if (heading0.defined()) {
      auto heading = heading0.get();
      auto bow = HorizontalMotion<T>(
          state.boatOverGround.value
          - state.currentOverGround.value);

      // We should compute the actual leeway angle and compare it
      // against the predicted leeway angle. The leeway angle is undefined,
      // I suppose, if the boat doesn't move across water. I think we
      // should somehow make a comparison which is scaled by
      // the boat velocity.

      Velocity<T> vs = bow.norm();
      if (Velocity<T>::knots(MakeConstant<T>::apply(0.0)) < vs) {
        Angle<T> predictedLeeway = k*state.heel.value/(vs*vs);

        Angle<T> trueLeeway = heading - bow.angle();

        Angle<T> angleError = (trueLeeway - predictedLeeway)
            .normalizedAt0();
        Velocity<T> velocityError = angleError.radians()*vs;
        residuals[0] = sqrtHuber<T>(
            velocityError/BandWidthForType<T, Velocity<T>>::get());
      }
    }
    return true;
  }
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
