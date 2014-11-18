/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BOATSIM_H_
#define BOATSIM_H_

namespace sail {

#pragma pack(push)
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

    double timeSeconds;
    double boatOrientationRadians;
    double boatMotionThroughWaterMPS;
    double rudderAngleRadians;
    double magneticHeadingRadians;
    double boatXMeters, boatYMeters;

    static constexpr int paramCount() {
      return sizeof(BoatSimulationState)/sizeof(double);
    }
  };
#pragma pack(pop)

}

#endif /* BOATSIM_H_ */
