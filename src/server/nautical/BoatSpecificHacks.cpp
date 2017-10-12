/*
 * BoatSpecificHacks.cpp
 *
 *  Created on: 6 Oct 2017
 *      Author: jonas
 */

#include "BoatSpecificHacks.h"
#include <server/common/logging.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/PositiveMod.h>
#include <server/common/DynamicScope.h>

namespace hack {


sail::Duration<double> timeSinceMidnight(double h, double m, double s) {
  using namespace sail;
  return h*1.0_hours + m*1.0_minutes + s*1.0_s;
}


sail::TimeStamp advanceTime(
    sail::TimeStamp last,
    int hour,
    int minute,
    int second) {
  using namespace sail;
  CHECK(last.defined());
  auto tm = last.makeGMTimeStruct();

  auto lastTms = timeSinceMidnight(tm.tm_hour, tm.tm_min, tm.tm_sec);
  auto newTms = timeSinceMidnight(hour, minute, second);
  auto toAdd = positiveMod<Duration<double>>(newTms - lastTms, 1.0_days);
  return last + toAdd;
}

bool forceDateForGLL = false;
bool performTimeGuessNow = false;
sail::TimeStamp nmea0183TimeGuess = sail::TimeStamp::UTC(2017, 9, 9, 0, 0, 0);
bool bootCountToDateHack = false;

void ConfigureForBoat(const std::string& boatId) {
  if (boatId == "59b1343a0411db0c8d8fbf7c") {
    hack::forceDateForGLL = true;
    hack::bootCountToDateHack = true;
  }
}

}
