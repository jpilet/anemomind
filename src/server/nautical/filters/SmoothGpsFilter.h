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

CeresTrajectoryFilter::Settings makeDefaultOptSettings();

struct GpsFilterSettings {
  CeresTrajectoryFilter::Settings ceresSettings = makeDefaultOptSettings();
  Duration<double> samplingPeriod = Duration<double>::seconds(1.0);
  Duration<double> subProblemThreshold = Duration<double>::minutes(3.0);
  Duration<double> subProblemLength = Duration<double>::hours(4.0);
};

struct GpsFilterResults {
  GeographicReference geoRef;
  Array<CeresTrajectoryFilter::Types<2>::TimedPosition>
    rawLocalPositions,
    filteredLocalPositions;

  bool empty() const {return filteredLocalPositions.empty();}
  TimedSampleCollection<GeographicPosition<double> >::TimedVector getGlobalPositions() const;
  TimedSampleCollection<HorizontalMotion<double> >::TimedVector getGpsMotions() const;
};

Array<TimeStamp> listSplittingTimeStamps(const Array<TimeStamp> &timeStamps,
    Duration<double> threshold);

GpsFilterResults filterGpsData(const NavDataset &ds,
    const GpsFilterSettings &settings = GpsFilterSettings());

template <typename T>
Array<Array<T> > applySplits(const Array<T> &src,
    const Array<TimeStamp> &splits);
}

#endif /* SERVER_NAUTICAL_FILTERS_SMOOTHGPSFILTER_H_ */
