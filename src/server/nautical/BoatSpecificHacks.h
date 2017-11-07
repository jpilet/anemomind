/*
 * BoatSpecificHacks.h
 *
 *  Created on: 6 Oct 2017
 *      Author: jonas
 */
#ifndef SERVER_NAUTICAL_BOATSPECIFICHACKS_H_
#define SERVER_NAUTICAL_BOATSPECIFICHACKS_H_

#include <string>

namespace sail {

class NavDataset;

namespace hack {

/*
 *  Sensei
 *
 */
// Used for Sensei (59b1343a0411db0c8d8fbf7c),
// because their GPS device does not produce
// full dates (only time of the day, not *which* day).
extern bool forceDateForGLL;

// Used to generate dates when the above is true.
extern int bootCount;

extern double motionWeight;



void ConfigureForBoat(const std::string& boatId);

void SelectSources(NavDataset *dataset);

}

} // namespace sail


#endif /* SERVER_NAUTICAL_BOATSPECIFICHACKS_H_ */
