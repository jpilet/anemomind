/*
 * BoatSpecificHacks.cpp
 *
 *  Created on: 6 Oct 2017
 *      Author: jonas
 */

#include "BoatSpecificHacks.h"
#include <server/nautical/NavDataset.h>

namespace sail {
namespace hack {

namespace {
std::string gBoatId;

const char kRealTeamD35[] = "5992fcc6035eb352cf36d594";
const char kRealTeamGC32[] = "5a6b312270d7bf3fce86b6ad";
}

bool forceDateForGLL = false;
int bootCount = 0;


void ConfigureForBoat(const std::string& boatId) {
  gBoatId = boatId;

  if (boatId == "59b1343a0411db0c8d8fbf7c") {
    // Sensei, issue #1146. Has no full dates, just time of day.
    // So we have to generate full dates somehow using bootcount
    // to make it look good.
    forceDateForGLL = true;
  }
  if (boatId == kRealTeamD35) {
    // Realteam, issue #1138. NMEA0183 buffering likely
    // leads to a slight staircase effect in the positions,
    // which in turn leads to a very noisy filtered GPS speed.
    // Giving more weight to the motion fixes that.
  }
}

void SelectSources(NavDataset *dataset) {
  if (gBoatId == kRealTeamD35) {
    // Realteam, issue #1138. NMEA0183 buffering likely
    dataset->dispatcher()->setSourcePriority("NMEA0183: /dev/ttyMFD1", -10);

  } else if (gBoatId == kRealTeamGC32) {
    // Take apparent wind from the processor, not the sensor (rotating mast).
    dataset->dispatcher()->setSourcePriority("NMEA2000/c078be002fb01596", 20);
  }
}

} // namespace hack

} // namespace sail
