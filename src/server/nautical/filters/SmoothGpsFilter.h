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

  TimedSampleCollection<GeographicPosition<double> > getGlobalPositions() const;
};

CeresTrajectoryFilter::Settings makeDefaultCeresGpsFilterSettings();

struct GpsFilterSettings {
  enum Backend {
    Ceres, Irls
  };

  CeresTrajectoryFilter::Settings ceresSettings = makeDefaultCeresGpsFilterSettings();
  Backend backend = Irls;
};

GpsFilterResults filterGpsData(const NavDataset &ds,
    const GpsFilterSettings &settings = GpsFilterSettings());

}

#endif /* SERVER_NAUTICAL_FILTERS_SMOOTHGPSFILTER_H_ */
