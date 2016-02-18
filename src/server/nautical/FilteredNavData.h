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
  FilteredNavData(NavCollection navs, double lambda, DebugPlotMode mode = NONE);

  FilteredNavData(
        TimeStamp timeOffset,
        LineKM sampling,
        Array<Angle<double> > awaSamples,
        Array<Angle<double> > magHdgSamples,
        Array<Angle<double> > gpsBearingSamples,
        Array<Velocity<double> > watSpeedSamples,
        Array<Velocity<double> > gpsSpeedSamples,
        Array<Velocity<double> > awsSamples);


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

  const LineKM &indexToX() const {
    return _awa.indexToX();
  }

  double samplingPeriod() const {
    return indexToX().getK();
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

  // Estimated standard deviations for the noise
  class NoiseStdDev {
   public:
    NoiseStdDev(
        Angle<double> awa_,
        Angle<double> magHdg_,
        Angle<double> gpsBearing_,
        Velocity<double> watSpeed_,
        Velocity<double> gpsSpeed_,
        Velocity<double> aws_) : awa(awa_),
        magHdg(magHdg_), gpsBearing(gpsBearing_),
        watSpeed(watSpeed_), gpsSpeed(gpsSpeed_), aws(aws_) {}

    Angle<double> awa;
    Angle<double> magHdg;
    Angle<double> gpsBearing;
    Velocity<double> watSpeed;
    Velocity<double> gpsSpeed;
    Velocity<double> aws;
  };
  NoiseStdDev estimateNoise(NavCollection navs) const;

  // The lifetime of _data should completely overlap
  // the lifetime of this object.
  class Indexed {
   public:
    Indexed() : _data(nullptr), _index(-1) {}

    Indexed(const FilteredNavData *data, int index) :
      _data(data), _index(index) {}

    Angle<double> awa() const {
      return _data->awa().get(_index);
    }

    Angle<double> magHdg() const {
      return _data->magHdg().get(_index);
    }

    Angle<double> gpsBearing() const {
      return _data->gpsBearing().get(_index);
    }

    Velocity<double> watSpeed() const {
      return _data->watSpeed().get(_index);
    }

    Velocity<double> gpsSpeed() const {
      return _data->gpsSpeed().get(_index);
    }

    Velocity<double> aws() const {
      return _data->aws().get(_index);
    }
   private:
    const FilteredNavData *_data;
    int _index;
  };

  Indexed makeIndexedInstrumentAbstraction(int sampleIndex) const {
    assert(0 <= sampleIndex);
    assert(sampleIndex < size());
    return Indexed(this, sampleIndex);
  }

  Array<FilteredNavData::Indexed> makeIndexedInstrumentAbstractions() const;

  TimeStamp timeOffset() const {
    return _timeOffset;
  }

  Array<Duration<double> > timesSinceOffset() const;
 private:
  TimeStamp _timeOffset;
  UniformSamples<Angle<double> > _awa, _magHdg, _gpsBearing;
  UniformSamples<Velocity<double> > _watSpeed, _gpsSpeed, _aws;
};

}

#endif /* FILTEREDNAVDATA_H_ */
