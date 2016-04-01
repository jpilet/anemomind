/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BOATSIM_H_
#define BOATSIM_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <memory>
#include <server/common/Array.h>
#include <server/common/Function.h>
#include <server/nautical/GeographicReference.h>
#include <iosfwd>

namespace sail {

#pragma pack(push)
  // This class characterizes the current state of the boat,
  // for any boat type.
  //
  // We use it to store both the state and the derivative of the state
  // w.r.t. time (in seconds).
  class BoatSimulationState {
   public:
    BoatSimulationState() {toArray().setTo(0);}

    Arrayd toArray() {
      return Arrayd(paramCount(), reinterpret_cast<double*>(this));
    }

    bool valid();



    /*
     * Accessors for reading the individual state variables.
     */
    Duration<double> time() const {
      return Duration<double>::seconds(timeSeconds);
    }

    Angle<double> boatOrientation() const {
      return Angle<double>::radians(boatOrientationRadians);
    }

     // Since we currently do not have a quantity Angle/Time,
     // we return it of type Angle (measured per second).
     // In future we might extend the PhysicalQuantity type to accomodate
     // such complex quantities.
     Angle<double> boatAngularVelocity() const {
       return Angle<double>::radians(boatAngularVelocityRadPerSec);
     }

     Velocity<double> boatSpeedThroughWater() const {
       return Velocity<double>::metersPerSecond(boatSpeedThroughWaterMPS);
     }

     Angle<double> rudderAngle() const {
       return Angle<double>::radians(rudderAngleRadians);
     }

     Length<double> boatX() const {
       return Length<double>::meters(boatXMeters);
     }

     Length<double> boatY() const {
       return Length<double>::meters(boatYMeters);
     }



    /*
     * Accessors for setting the derivatives of state variables.
     */
     void setTimeDeriv(double t = 1.0 /*Dimensionless, timeunit per timeunit*/) {
        timeSeconds = t;
     }

     void setBoatOrientationDeriv(Angle<double> anglePerSecond) {
       boatOrientationRadians = anglePerSecond.radians();
     }

     void setBoatAngularVelocityDeriv(Angle<double> anglePerSquaredSecond) {
       boatAngularVelocityRadPerSec = anglePerSquaredSecond.radians();
     }

     void setRudderAngleDeriv(Angle<double> anglePerSecond) {
       rudderAngleRadians = anglePerSecond.radians();
     }

     void setBoatSpeedThroughWaterDeriv(Velocity<double> velocityPerSecond) {
       boatSpeedThroughWaterMPS = velocityPerSecond.metersPerSecond();
     }

     void setBoatXDeriv(Velocity<double> x) {
       boatXMeters = x.metersPerSecond();
     }

     void setBoatYDeriv(Velocity<double> x) {
       boatYMeters = x.metersPerSecond();
     }

     static constexpr int paramCount() {
       return sizeof(BoatSimulationState)/sizeof(double);
     }

   private:
    // STATE VARIABLES:
    // I would like to use PhysicalQuantities here,
    // but I am not sure they are packed... If we should pack them,
    // we should do that in its own PR because we might break things.
    // However, having them as raw doubles has the advantage that we can use
    // the same class to store both the state and the derivatives
    // of the state variables.

    // The state of the time
    double timeSeconds;

    // In which direction the boat is pointing
    double boatOrientationRadians;

    // How fast the boat is turning
    double boatAngularVelocityRadPerSec;

    // How fast the boat moves through water
    double boatSpeedThroughWaterMPS;

    // By what angle the rudder is turned
    double rudderAngleRadians;

    // The boat position in a local coordinate system
    // Can be mapped to global coordinates using GeographicReference class.
    double boatXMeters, boatYMeters;


    /*
     * TODO:
     * Should we also model the inertia of mechanical details in the instruments,
     * such as the fact the it may take some time for the wind wheel to change speed if
     * there is a rapid drop in wind? Or are these things too fine to model?
     */
  };
#pragma pack(pop)

// This class describes the characteristics of a boat, according to our model.
// Sensible default values are provided that can be overridden as desired.
class BoatCharacteristics {
 public:
  BoatCharacteristics() :
    keelRudderDistance(Length<double>::meters(2.0)),
    targetSpeedFun(&defaultTargetSpeed),
    rudderResistanceCoef(0.1),
    halfTargetSpeedTime(Duration<double>::seconds(3.0)),
    rudderCorrectionCoef(2.0),
    rudderMaxAngle(Angle<double>::degrees(15)),
    rudderFineTune(Angle<double>::degrees(1)),
    correctionThreshold(Angle<double>::degrees(5)),
    boatReactiveness(20.0)
    {}

  // The distance between the keel and the rudder.
  Length<double> keelRudderDistance;

  // How fast the boat moves forward, given true wind and current.
  std::function<Velocity<double>(Angle<double>,Velocity<double>)> targetSpeedFun;




  // How much the rudder slows down the boat as the rudder angle increases.
  double rudderResistanceCoef;

  // The time it takes to reach half of its target speed,
  // if the rudder angle is not changing.
  Duration<double> halfTargetSpeedTime;

  // How fast the helmsman will turn the rudder towards
  // its max position. For instance, if the coefficient is 1 and the difference
  // between the current rudder angle and its target position is 49 degrees,
  // then he will at that time instant turn the rudder with an angular velocity
  // of 49 degrees per second. If the coefficient would be 0.5, he would turn the rudder
  // with a velocity of 24 degrees per second.
  double rudderCorrectionCoef;

  // In addition to moving the rudder towards a target angle,
  // the helmsman also tries to push the rudder at this constant
  // angle per second in order for the error to be asymptotically close to 0.
  Angle<double> rudderFineTune;


  // How rapidly the boat reacts when the rudder is turned
  double boatReactiveness;

  typedef std::shared_ptr<BoatCharacteristics> Ptr;

  // The maximum angle at which the helmsman will turn the rudder.
  Angle<double> rudderMaxAngle;

  // The maximum angle error from the target value that
  // the helmsman tolerates. When the error is below this angle,
  // he will keep the rudder in its middle position. If the error goes
  // above this threshold, he will start to turn the rudder towards its
  // minimum/maximum angle ( rudderMaxAngle() ) with a rate of rudderCorrectionCoef()
  Angle<double> correctionThreshold;


  // Helper methods
  static Velocity<double> defaultTargetSpeed(Angle<double> twa, Velocity<double> tws);

  // How fast the boat reaches its target speed.
  double targetSpeedGain() const {
    // clang does not like log(2.0) as constexpr.
    constexpr double log2 = 0.693147180559945309417232121458;
    return log2/halfTargetSpeedTime.seconds();
  }

};

class BoatSim : public Function {
 public:
  typedef GeographicReference::ProjectedPosition ProjectedPosition;
  typedef HorizontalMotion<double> FlowVector;
  typedef std::function<FlowVector(ProjectedPosition, Duration<double>)> FlowFun;


  // Contains lots of information that can be derived from the environment
  // and a BoatSimState. This class contains values that can be used to build
  // test cases.
  class FullState {
   public:
    FullState() {}

    Angle<double> rudderAngle;
    ProjectedPosition pos;
    Duration<double> time;
    Angle<double> boatOrientation;
    Angle<double> boatAngularVelocity;
    Velocity<double> boatSpeedThroughWater;
    HorizontalMotion<double> trueWind;
    HorizontalMotion<double> trueCurrent;
    HorizontalMotion<double> windWrtCurrent;
    Angle<double> windAngleWrtWater;
    Velocity<double> windSpeedWrtWater;
    HorizontalMotion<double> boatMotionThroughWater;
    HorizontalMotion<double> boatMotion;

    HorizontalMotion<double> apparentWind() const {
      return trueWind - boatMotion;
    }

    Angle<double> awa() const {
      return apparentWind().angle() - Angle<double>::degrees(180) - boatOrientation;
    }
  };

  // The desired TWA at duration since the simulation starts.
  typedef std::function<Angle<double>(Duration<double>)> TwaFunction;

  BoatSim(
      FlowFun windFun,
      FlowFun currentFun,
      BoatCharacteristics ch,
      TwaFunction twaFunction);

  int inDims() {return BoatSimulationState::paramCount();}
  int outDims() {return BoatSimulationState::paramCount();}

  FullState makeFullState(const BoatSimulationState &state);

  // Evaluates the derivatives of a state
  void eval(double *Xin, double *Fout, double *Jout);

  Array<FullState> simulate(Duration<double> simulationDurationti,
    Duration<double> samplingPeriod, int iterationsPerSample);

  // For conveniency in synthesizing test data.
  static TwaFunction makePiecewiseTwaFunction(
      Array<Duration<double> > durs,
      Array<Angle<double> > twa);

  static void makePlots(Array<BoatSim::FullState> states);
 private:
  FlowFun _windFun;
  FlowFun _currentFun;
  BoatCharacteristics _ch;
  TwaFunction _twaFunction;
};

std::ostream &operator<<(std::ostream &s,
    const BoatSim::FullState &state);

}

#endif /* BOATSIM_H_ */
