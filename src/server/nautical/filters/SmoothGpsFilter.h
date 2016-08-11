/*
 * SmoothGPSFilter.h
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_FILTERS_SMOOTHGPSFILTER_H_
#define SERVER_NAUTICAL_FILTERS_SMOOTHGPSFILTER_H_

#include <server/math/nonlinear/CeresTrajectoryFilter.h>
#include <server/nautical/GeographicReference.h>
#include <server/nautical/NavDataset.h>
#include <server/nautical/GeographicReference.h>

namespace sail {

struct GpsFilterResults {
  GeographicReference geoRef;
  Array<CeresTrajectoryFilter::Types<2>::TimedPosition>
    rawLocalPositions,
    filteredLocalPositions;

  bool empty() const {return filteredLocalPositions.empty();}
  TimedSampleCollection<GeographicPosition<double> >::TimedVector getGlobalPositions() const;
  TimedSampleCollection<HorizontalMotion<double> >::TimedVector getGpsMotions() const;
};

CeresTrajectoryFilter::Settings makeDefaultSettings();

GpsFilterResults filterGpsData(const NavDataset &ds,
    const CeresTrajectoryFilter::Settings &settings
	= makeDefaultSettings());

}

#endif /* SERVER_NAUTICAL_FILTERS_SMOOTHGPSFILTER_H_ */
