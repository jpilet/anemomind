/*
 * GpsUtils.h
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_FILTERS_GPSUTILS_H_
#define SERVER_NAUTICAL_FILTERS_GPSUTILS_H_

#include <server/common/TimedValue.h>
#include <server/nautical/NavDataset.h>
#include <server/nautical/GeographicReference.h>
#include <server/math/nonlinear/TimedObservation.h>

namespace sail {
namespace GpsUtils {

Array<TimedValue<HorizontalMotion<double> > > getGpsMotions(const NavDataset &ds);

GeographicPosition<double> getReferencePosition(
    const TimedSampleRange<GeographicPosition<double> > &positions);

TimeStamp getReferenceTime(
    const TimedSampleRange<GeographicPosition<double> > &positions);


// All numbers in SI units
Array<TimedObservation<2> > toLocalObservations(
    const GeographicReference &geoRef, TimeStamp timeReference,
    const TimedSampleRange<GeographicPosition<double> > &positions);

Array<TimedObservation<2> > toLocalObservations(TimeStamp timeReference,
    const Array<TimedValue<HorizontalMotion<double> > > &motions);

}
}

#endif /* SERVER_NAUTICAL_FILTERS_GPSUTILS_H_ */
