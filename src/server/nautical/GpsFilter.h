/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_GPSFILTER_H_
#define SERVER_NAUTICAL_GPSFILTER_H_

#include <server/nautical/Nav.h>
#include <server/math/nonlinear/BandedSolver.h>
#include <server/nautical/GeographicReference.h>

namespace sail {
namespace GpsFilter {

struct Settings {
  Settings();
  Duration<double> samplingPeriod;
  BandedSolver::Settings filterSettings;
};

struct Results {
  // Before filtering
  Array<Nav> rawNavs;

  // Input to the filtering algorithm. Twice as many elements as there are navs.
  Array<Observation<2> > observations;

  // Related to the optimization
  Sampling sampling;
  MDArray2d Xmeters;
  TimeStamp timeRef;
  GeographicReference geoRef;

  Array<Nav> filteredNavs() const;
};

Results filter(Array<Nav> navs, Settings settings);

}
}

#endif /* SERVER_NAUTICAL_GPSFILTER_H_ */
