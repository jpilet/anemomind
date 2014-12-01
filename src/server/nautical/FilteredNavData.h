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

  void setAwa(const UniformSamples<Angle<double> > &awa) {
    _awa = awa;
  }

  const UniformSamples<Angle<double> > &magHdg() const {
    return _magHdg;
  }

  void setMagHdg(const UniformSamples<Angle<double> > &magHdg) {
    _magHdg = magHdg;
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

  const LineKM &sampling() const {
    return _awa.sampling();
  }

  double samplingPeriod() const {
    return sampling().getK();
  }

  int size() const {
    return _awa.size();
  }

  Arrayd makeCenteredX() const;

  HorizontalMotion<double> gpsMotion(double localTime) const;
  HorizontalMotion<double> gpsMotionAtIndex(int index) const;

  double low() const {
    return _awa.low();
  }

  double high() const {
    return _awa.high();
  }
 private:
  TimeStamp _timeOffset;
  UniformSamples<Angle<double> > _awa, _magHdg, _gpsBearing;
  UniformSamples<Velocity<double> > _watSpeed, _gpsSpeed, _aws;
};

}

#endif /* FILTEREDNAVDATA_H_ */
