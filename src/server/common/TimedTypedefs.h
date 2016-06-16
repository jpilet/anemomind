/*
 * TimedTypedefs.h
 *
 *  Created on: Jun 10, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TIMEDTYPEDEFS_H_
#define SERVER_COMMON_TIMEDTYPEDEFS_H_

#include <server/common/TimedValue.h>
#include <server/nautical/GeographicPosition.h>

namespace sail {

typedef TimedValue<GeographicPosition<double> > TimedGeoPos;

}



#endif /* SERVER_COMMON_TIMEDTYPEDEFS_H_ */
