/*
 * GpsUtils.h
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_FILTERS_GPSUTILS_H_
#define SERVER_NAUTICAL_FILTERS_GPSUTILS_H_

#include <server/common/TimedValue.h>
#include <server/nautical/GeographicReference.h>
#include <server/nautical/NavDataset.h>

namespace sail {
namespace GpsUtils {

HorizontalMotion<double> computeHorizontalMotion(
    const TimedValue<GeographicPosition<double>> &from,
    const TimedValue<GeographicPosition<double>> &to);

Array<TimedValue<HorizontalMotion<double> > > getGpsMotions(const NavDataset &ds);

GeographicPosition<double> getReferencePosition(
    const TimedSampleRange<GeographicPosition<double> > &positions);

TimeStamp getReferenceTime(
    const TimedSampleRange<GeographicPosition<double> > &positions);

}
}

#endif /* SERVER_NAUTICAL_FILTERS_GPSUTILS_H_ */
