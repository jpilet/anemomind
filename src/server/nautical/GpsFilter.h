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
  double motionWeight;
  BandedSolver::Settings filterSettings;
  bool useCeres = false;
};

struct Results {
  // Before filtering
  Array<Nav> rawNavs;
  Array<Observation<2> > positionObservations;

  // Related to the optimization
  Sampling sampling;
  MDArray2d Xmeters;
  TimeStamp timeRef;
  GeographicReference geoRef;

  Array<Nav> filteredNavs() const;
  Sampling::Weights calcWeights(TimeStamp t) const;
  HorizontalMotion<double> calcMotion(const Sampling::Weights &w) const;
  GeographicPosition<double> calcPosition(const Sampling::Weights &w) const;
};

Results filter(Array<Nav> navs, Settings settings);

}
}

#endif /* SERVER_NAUTICAL_GPSFILTER_H_ */
