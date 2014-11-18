/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BOATSIM_H_
#define BOATSIM_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <memory>
#include <server/math/nonlinear/RungeKutta.h>
#include <server/common/Span.h>
#include <server/common/ProportionateIndexer.h>

namespace sail {

#pragma pack(push)
  // This class characterizes the current state of the boat,
  // for any boat type.
  class BoatSimulationState {
   public:
    BoatSimulationState() :
      timeSeconds(0),
      boatOrientationRadians(0),
      boatSpeedThroughWaterMPS(0),
      rudderAngleRadians(0),
      boatXMeters(0), boatYMeters(0) {}


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

    // How fast the boat moves through water
    double boatSpeedThroughWaterMPS;

    // By what angle the rudder is turned
    double rudderAngleRadians;

    // The boat position in a local coordinate system
    double boatXMeters, boatYMeters;

    static constexpr int paramCount() {
      return sizeof(BoatSimulationState)/sizeof(double);
    }
  };
#pragma pack(pop)

// This class describes the characteristics of a boat, according to our model.
// Sensible default values are provided that can be overridden as desired.
class BoatCharacteristics {
 public:
  BoatCharacteristics() :
    keelRudderDistance(Length<double>::meters(2.0)),
    targetSpeedFun(&defaultTargetSpeed),
    rudderResistanceCoef(0),
    halfTargetSpeedTime(Duration<double>::seconds(4.0)),
    rudderCorrectionCoef(0.5),
    rudderMaxAngle(Angle<double>::degrees(45)),
    correctionThreshold(Angle<double>::degrees(5))
    {}

  // The distance between the keel and the rudder.
  Length<double> keelRudderDistance;

  // How fast the boat moves forward, given true wind and current.
  std::function<Velocity<double>(Angle<double>,Velocity<double>)> targetSpeedFun;




  // How much the rudder slows down the boat as the rudder angle increases.
  // By default, the boat does not slow down at all, so tune this parameter
  // through experimentation to get good results.
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
    constexpr double log2 = log(2.0);
    return log2/halfTargetSpeedTime.seconds();
  }

};

class BoatSimulator : public Function {
 public:
  typedef HorizontalMotion<double> FlowVector;
  typedef std::function<FlowVector(Length<double>, Length<double>, Duration<double>)> FlowFun;


  // Contains lots of information that can be derived from the environment
  // and a BoatSimState. This class contains values that can be used to build
  // test cases.
  class FullBoatState {
   public:
    FullBoatState() {}

    Angle<double> rudderAngle;
    Length<double> x;
    Length<double> y;
    Duration<double> time;
    Angle<double> boatOrientation;
    Velocity<double> boatSpeedThroughWater;
    HorizontalMotion<double> trueWind;
    HorizontalMotion<double> trueCurrent;
    HorizontalMotion<double> windWrtCurrent;
    Angle<double> twaWater;
    Velocity<double> twsWater;
    HorizontalMotion<double> boatMotionThroughWater;
    HorizontalMotion<double> boatMotion;
  };

  /*
   * An array of TWASpans specify how the boat should be
   * steered. The helmsman tries to make the TWA of the boat
   * correspond to targetTWA.
   */
  class TwaDirective {
   public:
    Duration<double> duration;
    Angle<double> targetTwa;
  };

  BoatSimulator(
      FlowFun windFun,
      FlowFun currentFun,
      BoatCharacteristics::Ptr ch,
      Array<TwaDirective> twaSpans);

  int inDims() {return BoatSimulationState::paramCount();}
  int outDims() {return BoatSimulationState::paramCount();}

  FullBoatState makeFullState(const BoatSimulationState &state);

  void eval(double *Xin, double *Fout, double *Jout);
 private:
  FlowFun _windFun;
  FlowFun _currentFun;
  BoatCharacteristics::Ptr _ch;
  Array<TwaDirective> _twaSpans;
  ProportionateIndexer _indexer;
  Angle<double> getTargetTwa(Duration<double> time) const {
    return _twaSpans[_indexer.get(time.seconds())].targetTwa;
  }
};

}

#endif /* BOATSIM_H_ */
