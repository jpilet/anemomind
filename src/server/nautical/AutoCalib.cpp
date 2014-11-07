/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/Function.h>
#include <server/nautical/AutoCalib.h>

namespace sail {

namespace {
  class FilteredNavInstrumentAbstraction {
   public:
    FilteredNavInstrumentAbstraction(const FilteredNavData &data, int index) :
      _data(data), _index(index) {}

    Angle<double> awa() const {
      return _data.awa().get(_index);
    }

    Angle<double> magHdg() const {
      return _data.magHdg().get(_index);
    }

    Velocity<double> aws() const {
      return _data.aws().get(_index);
    }

    Velocity<double> watSpeed() const {
      return _data.watSpeed().get(_index);
    }

    Angle<double> gpsBearing() const {
      return _data.gpsBearing().get(_index);
    }

    Velocity<double> gpsSpeed() const {
      return _data.gpsSpeed().get(_index);
    }
   private:
    const FilteredNavData &_data;
    int _index;
  };

  class Objf : public Function {
   public:
    Objf(FilteredNavData data, Arrayd times, AutoCalib::Settings s);

    template <typename T>
    class Difs {
     public:
      Difs(T wd, T cd) : windDif(wd), currentDif(cd) {}
      T windDif;
      T currentDif;
    };

    int length() const {
      return _data.size();
    }

    int inDims() {
      return Corrector<double>::paramCount();
    }

    int outDims() {
      return 6*length();
    }


   private:
    AutoCalib::Settings _settings;
    FilteredNavData _data;
    Arrayd _times;
    double _qw, _qc;

    void adjustQualityParameters();


    int calcLowerIndex(int timeIndex) const {
      return int(floor(_data.sampling().inv(_times[timeIndex])));
    }

    double normGDeriv(int timeIndex) const;

    template <typename T>
    Difs<T> calcDifs(const Corrector<T> &corrector, int timeIndex) {
      int lowerIndex = calcLowerIndex(timeIndex);
      int upperIndex = lowerIndex + 1;

      CalibratedNav<T> a = corrector.correct(FilteredNavInstrumentAbstraction(_data, lowerIndex));
      CalibratedNav<T> b = corrector.correct(FilteredNavInstrumentAbstraction(_data, upperIndex));

      HorizontalMotion<T> wdif = b.trueWind() - a.trueWind();
      HorizontalMotion<T> cdif = b.trueCurrent() - a.trueCurrent();
      double factor = 1.0/_data.samplingPeriod();
      return Difs<T>(factor*wdif.norm().knots(), factor*cdif.norm().knots());
    }

    Difs<double> provokeCompileError() {
      Corrector<double> corr;
      int timeIndex = 0;
      return calcDifs<double>(corr, timeIndex);
    }
  };

  Objf::Objf(FilteredNavData data, Arrayd times, AutoCalib::Settings s) :
      _data(data), _times(times), _qw(s.wind.fixedQuality),
      _qc(s.current.fixedQuality), _settings(s) {

  }

  double Objf::normGDeriv(int timeIndex) const {
    int lowerIndex = calcLowerIndex(timeIndex);
    HorizontalMotion<double> a = _data.gpsMotionAtIndex(lowerIndex + 0);
    HorizontalMotion<double> b = _data.gpsMotionAtIndex(lowerIndex + 1);
    auto dif = b - a;
    return sqrt(sqr(dif[0].knots()) + sqr(dif[1].knots()))/_data.samplingPeriod();
  }
}

AutoCalib::Results AutoCalib::calibrate(FilteredNavData data, Arrayd times) const {
  if (times.empty()) {
    times = data.makeCenteredX();
  }

  Arrayd params = Corrector<double>().toArray().dup();

}


}
