/*
 * BoatSpecificHacks.h
 *
 *  Created on: 6 Oct 2017
 *      Author: jonas
 */
#ifndef SERVER_NAUTICAL_BOATSPECIFICHACKS_H_
#define SERVER_NAUTICAL_BOATSPECIFICHACKS_H_

#include <string>
#include <server/common/TimeStamp.h>

namespace hack {

sail::TimeStamp advanceTime(
    sail::TimeStamp last,
    int hour,
    int minute,
    int second);

/*
 *  Sensei
 *
 */
// Used for Sensei (59b1343a0411db0c8d8fbf7c),
// because their GPS device does not produce
// full dates (only time of the day, not *which* day).
extern bool forceDateForGLL;
extern bool performTimeGuessNow;

// Used to generate dates when the above is true.
extern sail::TimeStamp nmea0183TimeGuess;

extern bool bootCountToDateHack;



void ConfigureForBoat(const std::string& boatId);

}

#endif /* SERVER_NAUTICAL_BOATSPECIFICHACKS_H_ */
