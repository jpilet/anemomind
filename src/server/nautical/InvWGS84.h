/*
 * XYZ2WGS84.h
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_INVWGS84_H_
#define SERVER_NAUTICAL_INVWGS84_H_

#include <server/nautical/GeographicPosition.h>

namespace sail {

GeographicPosition<double> computeGeographicPositionFromXYZ(const Length<double> *xyz3);

}

#endif /* SERVER_NAUTICAL_INVWGS84_H_ */
