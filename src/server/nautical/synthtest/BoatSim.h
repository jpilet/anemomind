/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BOATSIM_H_
#define BOATSIM_H_

namespace sail {

#pragma pack(push)
  // This class characterizes the current state of the boat,
  // for any boat type.
  class BoatSimulationState {
   public:
    BoatSimulationState() :
      timeSeconds(0),
      boatOrientationRadians(0),
      boatMotionThroughWaterMPS(0),
      rudderAngleRadians(0),
      magneticHeadingRadians(0),
      boatXMeters(0), boatYMeters(0) {}


    // STATE VARIABLES: I would like to use PhysicalQuantities here,
    // but I am not sure they are packed... If we should pack them,
    // we should do that in its own PR because we might break things.

    // The state of the time
    double timeSeconds;

    // In which direction the boat is pointing
    double boatOrientationRadians;

    // How fast the boat moves through water
    double boatMotionThroughWaterMPS;

    // By what angle the rudder is turned
    double rudderAngleRadians;

    double magneticHeadingRadians;
    double boatXMeters, boatYMeters;

    static constexpr int paramCount() {
      return sizeof(BoatSimulationState)/sizeof(double);
    }
  };
#pragma pack(pop)

// This class describes the characteristics of a boat, according to our model.
class BoatCharacteristics {
 public:
  // The distance between the keel and the rudder.
  // This is used to estimate how fast the heading of the boat changes as
    virtual Length<double> keelRudderDistance() const = 0;

  // How fast the boat moves forward, given true wind and
  virtual Velocity<double> targetSpeed(
      Angle<double> twa, Velocity<double> tws) const = 0;

  // How much the rudder slows down the boat as the rudder angle increases.
  virtual double rudderResistanceCoef() const = 0;

  // How fast the boat reaches its target speed.
  virtual double targetSpeedGain() const;

  // How fast we turn the rudder to maintain the TWA
  virtual double rudderCorrectionCoef() const;

  typedef std::shared_ptr<BoatCharacteristics> Ptr;

  virtual ~BoatCharacteristics() {}
};

class BoatSimulator : public Function {
 public:
  typedef HorizontalMotion<double> FlowVector;
  typedef std::function<FlowVector(Length<double>, Length<double>, Duration<double>)> FlowFun;

  /*
   * An array of TWASpans specify how the boat should be
   * steered. The helmsman tries to make the TWA of the boat
   * correspond to targetTWA during timeSpan.
   */
  class TWASpan {
   public:
    Span<Duration<double> > timeSpan;

    // The TWA that the boat should
    // strive to maintain during 'timeSpan'
    Angle<double> targetTWA;
  };

  BoatSimulator(
      FlowFun windFun,
      FlowFun currentFun,
      BoatCharacteristics::Ptr ch,
      Array<TWASpan> twaSpans);

  int inDims() {return BoatSimulationState::paramCount();}
  int outDims() {return BoatSimulationState::paramCount();}

  void eval(double *Xin, double *Fout, double *Jout);

 private:

};

}

#endif /* BOATSIM_H_ */
