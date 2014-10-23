/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FILTEREDNAVDATA_H_
#define FILTEREDNAVDATA_H_

#include <server/common/TimeStamp.h>
#include <server/math/UniformSamples.h>
#include <server/nautical/Nav.h>

namespace sail {

class FilteredNavData {
 public:
  enum DebugPlotMode {NONE, SIGNAL, DERIVATIVE};

  FilteredNavData() {}
  FilteredNavData(Array<Nav> navs, double lambda, DebugPlotMode mode = NONE);

  const UniformSamples<Angle<double> > &awa() const {
    return _awa;
  }

  const UniformSamples<Angle<double> > &magHdg() const {
    return _magHdg;
  }

  const UniformSamples<Angle<double> > &gpsBearing() const {
    return _gpsBearing;
  }

  const UniformSamples<Velocity<double> > &watSpeed() const {
    return _watSpeed;
  }

  const UniformSamples<Velocity<double> > &gpsSpeed() const {
    return _gpsSpeed;
  }

  const UniformSamples<Velocity<double> > &aws() const {
    return _aws;
  }
 private:
  TimeStamp _timeOffset;
  UniformSamples<Angle<double> > _awa, _magHdg, _gpsBearing;
  UniformSamples<Velocity<double> > _watSpeed, _gpsSpeed, _aws;
};

}

#endif /* FILTEREDNAVDATA_H_ */
