/*
 * BoatSpecificHacks.cpp
 *
 *  Created on: 6 Oct 2017
 *      Author: jonas
 */

#include "BoatSpecificHacks.h"

namespace hack {

bool forceDateForGLL = false;
int bootCount = 0;
double motionWeight = 1.0;

void ConfigureForBoat(const std::string& boatId) {
  if (boatId == "59b1343a0411db0c8d8fbf7c") {
    // Sensei, issue #1146. Has no full dates, just time of day.
    // So we have to generate full dates somehow using bootcount
    // to make it look good.
    forceDateForGLL = true;
  }
  if (boatId == "5992fcc6035eb352cf36d594") {
    // Realteam, issue #1138. NMEA0183 buffering likely
    // leads to a slight staircase effect in the positions,
    // which in turn leads to a very noisy filtered GPS speed.
    // Giving more weight to the motion fixes that.
    motionWeight = 10.0;
  }
}

}
