/*
 * Ecef.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef ECEF_H_
#define ECEF_H_

#include <server/common/math.h>

namespace sail
{

#define ECEFA 6378137
#define ECEFE 8.1819190842622e-2




template <typename T>
void lla2ecef(T lonRadians, T latRadians, T altMetres, T &outX, T &outY, T &outZ)
{

	T N = ECEFA/sqrt(1 - sqr(ECEFE)*sqr(sin(latRadians)));

	outX = (N+altMetres) * cos(latRadians) * cos(lonRadians);
	outY = (N+altMetres) * cos(latRadians) * sin(lonRadians);
	outZ = ((1-sqr(ECEFE)) * N + altMetres) * sin(latRadians);
}



} /* namespace sail */

#endif /* ECEF_H_ */
