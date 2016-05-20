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

namespace sail {

struct Results {
  GeographicReference geoRef;
  Array<CeresTrajectoryFilter::Types<2>::TimedPosition> localPositions;
  Array<TimedValue<GeographicPosition<double> > > getGlobalPositions() const;
};

Results filterGpsData(const NavDataset &ds);

}

#endif /* SERVER_NAUTICAL_FILTERS_SMOOTHGPSFILTER_H_ */
