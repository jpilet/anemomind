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
extern bool bootCountToDateHack = false;

void ConfigureForBoat(const std::string& boatId) {
  if (boatId == "59b1343a0411db0c8d8fbf7c") {
    hack::forceDateForGLL = true;
    hack::bootCountToDateHack = true;
  }
}

}
