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

Array<TimeStamp> listSplittingTimeStamps(const Array<TimeStamp> &timeStamps,
    Duration<double> threshold);

GpsFilterResults filterGpsData(const NavDataset &ds,
    const CeresTrajectoryFilter::Settings &settings = makeDefaultSettings());

template <typename T>
Array<Array<T> > applySplits(const Array<T> &src,
    const Array<TimeStamp> &splits);
}

#endif /* SERVER_NAUTICAL_FILTERS_SMOOTHGPSFILTER_H_ */
