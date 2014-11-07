/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/armaadolc.h>
#include <server/common/Function.h>
#include <server/nautical/AutoCalib.h>
#include <adolc/taping.h>

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
    static constexpr int blockSize = 6;
    Objf(FilteredNavData data, Arrayd times, AutoCalib::Settings s);

    template <typename T>
    class Difs {
     public:
      Difs(HorizontalMotion<T> wd, HorizontalMotion<T> cd) :
        windDif(wd), currentDif(cd) {}
      HorizontalMotion<T> windDif;
      HorizontalMotion<T> currentDif;
    };

    int length() const {
      return _data.size();
    }

    int inDims() {
      return Corrector<double>::paramCount();
    }

    int outDims() {
      return blockSize*length();
    }

    void eval(double *X, double *F, double *J);
   private:
    AutoCalib::Settings _settings;
    FilteredNavData _data;
    Arrayd _times, _G;
    Arrayd _tempW, _tempC;
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
      T factor = T(1.0/_data.samplingPeriod());
      return Difs<T>(factor*wdif, factor*cdif);
    }

    void evalSub(int index, double *X, double *F, MDArray2d J);
  };

  template <typename T>
  T unwrap(Velocity<T> x) {
    return x.knots();
  }

  double calcSigma(double g, double quality) {
    return sqrt(g/sqr(quality));
  }

  Objf::Objf(FilteredNavData data, Arrayd times, AutoCalib::Settings s) :
      _data(data), _times(times), _qw(s.wind.fixedQuality),
      _qc(s.current.fixedQuality), _settings(s), _G(times.size()),
      _tempW(times.size()), _tempC(times.size()) {
      for (int i = 0; i < length(); i++) {
        _G[i] = normGDeriv(i);
      }
  }

  double Objf::normGDeriv(int timeIndex) const {
    int lowerIndex = calcLowerIndex(timeIndex);
    if (_settings.gMode == AutoCalib::Settings::GPS) {
      HorizontalMotion<double> a = _data.gpsMotionAtIndex(lowerIndex + 0);
      HorizontalMotion<double> b = _data.gpsMotionAtIndex(lowerIndex + 1);
      auto dif = b - a;
      return sqrt(sqr(dif[0].knots()) + sqr(dif[1].knots()))/_data.samplingPeriod();
    } else {
      return std::abs(_data.magHdg().interpolateLinearDerivative(
          _data.sampling()(_times[timeIndex])).degrees());
    }
  }

  void Objf::eval(double *X, double *F, double *J) {
    bool outputJ = J != nullptr;
    MDArray2d JMat;
    if (outputJ) {
      JMat = MDArray2d(outDims(), inDims(), J);
    }
    for (int i = 0; i < length(); i++) {
      evalSub(i, X, F + blockSize*i, (outputJ? JMat.sliceRowBlock(i, blockSize) : MDArray2d()));
    }
  }


  double evalRobust(bool smooth, double quality, const HorizontalMotion<adouble> &X, double g, adouble *result) {
    double sigma = calcSigma(g, quality);
    adouble x = X.norm().knots();
    double xd = x.getValue();
    bool inlier = (xd < sigma);
    adouble lambda = (smooth?
                        sqr(sigma)/(sqr(sigma) + sqr(x))
                        : adouble(inlier? 1 : 0));
    if (std::isnan(lambda.getValue())) { // Could occur if smooth=true, g=0 and x=0
      lambda = 0;
    }
    adouble ql = quality*lambda;
    result[0] = ql*X[0].knots();
    result[1] = ql*X[1].knots();
    result[2] = quality*(1.0 - lambda)*sigma;
    return xd;
  }

  void Objf::evalSub(int index, double *X, double *F, MDArray2d J) {
    double g = _G[index];
    Array<adouble> adX = adolcInput(inDims(), X);
    bool outputJ = !J.empty();
    Corrector<adouble> *corr = Corrector<adouble>::fromArray(adX);

    if (outputJ) {
      trace_on(_settings.tapeIndex);
    }
    adouble result[blockSize]; {
      Difs<adouble> difs = calcDifs<adouble>(*corr, index);
      _tempW[index] = evalRobust(_settings.smooth, _qw, difs.windDif,    g, result + 0);
      _tempC[index] = evalRobust(_settings.smooth, _qc, difs.currentDif, g, result + 3);
    } adolcOutput(outDims(), result, F);
    if (outputJ) {
      trace_off();
      outputJacobianColMajor(_settings.tapeIndex, X, J.ptr(), J.getStep());
    }

  }
}

AutoCalib::Results AutoCalib::calibrate(FilteredNavData data, Arrayd times) const {
  if (times.empty()) {
    times = data.makeCenteredX();
  }

  Arrayd params = Corrector<double>().toArray().dup();

}


}
