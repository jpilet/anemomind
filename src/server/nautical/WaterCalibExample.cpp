/*
 *  Created on: 24 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/FilteredNavs.h>
#include <server/math/ADFunction.h>
#include <server/common/Array.h>
#include <server/nautical/GeoCalc.h>
#include <server/common/string.h>
#include <server/nautical/SpeedCalib.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/nonlinear/LevmarSettings.h>


namespace sail {

class WaterObjf : public AutoDiffFunction {
 public:
  WaterObjf(GeographicReference ref, Array<Nav> navs,
      FilteredNavs fnavs);

  // [Magnetic offset] + [SpeedCalib]
  int inDims() {return 1 + 4;}
  int outDims() {return 2*_inds.size();}
  void evalAD(adouble *Xin, adouble *Fout);

  Arrayd makeInitialParams();
  template <typename T> T &magOffset(T *x) {return x[0];}
  template <typename T> T &k(T *x) {return x[1];}
  template <typename T> T &m(T *x) {return x[2];}
  template <typename T> T &c(T *x) {return x[3];}
  template <typename T> T &alpha(T *x) {return x[4];}

  void disp(Arrayd params);
 private:
  Arrayi _inds;

  GeographicReference _geoRef;
  Array<Nav> _navs;
  FilteredNavs _fnavs;

};


template <typename T>
T calcDx(T mv, T md, T av, T ad) {
  return mv*cos(av)*ad + md*sin(av);
}

double calcDxSig(FilteredSignal mag, FilteredSignal angle, double t) {
  double mv = mag.value(t);
  double md = mag.derivative(t);
  double av = angle.value(t);
  double ad = angle.derivative(t);
  return calcDx(mv, md, av, ad);
}




template <typename T>
T calcDy(T mv, T md, T av, T ad) {
  return -mv*sin(av)*ad + md*cos(av);
}

double calcDySig(FilteredSignal mag, FilteredSignal angle, double t) {
  double mv = mag.value(t);
  double md = mag.derivative(t);
  double av = angle.value(t);
  double ad = angle.derivative(t);
  return calcDy(mv, md, av, ad);
}


void WaterObjf::evalAD(adouble *Xin, adouble *Fout) {
  int count = _inds.size();
  for (int I = 0; I < count; I++) {
    int i = _inds[I];

    double t = _fnavs.times[i].seconds();
    adouble *f = Fout + 2*I;
    SpeedCalib<adouble> calib(k(Xin), m(Xin), c(Xin), alpha(Xin));

    adouble measuredWatSpeed = _fnavs.watSpeed.value(t);

    // Estimated true magnitude of water speed
    adouble r = calib.eval(measuredWatSpeed);
    adouble drdt = calib.evalDeriv(measuredWatSpeed)*_fnavs.watSpeed.derivative(t);

    // Estimated true angle of water
    adouble av = _fnavs.magHdg.value(t) + magOffset(Xin);
    adouble ad = _fnavs.magHdg.derivative(t);

    // Derivatives of gps speed. Constant w.r.t. parameters
    double dvxdt = calcDxSig(_fnavs.gpsSpeed, _fnavs.gpsBearing, t);
    double dvydt = calcDySig(_fnavs.gpsSpeed, _fnavs.gpsBearing, t);

    adouble dwxdt = calcDx(r, drdt, av, ad);
    adouble dwydt = calcDy(r, drdt, av, ad);
    f[0] = dwxdt - dvxdt;
    f[1] = dwydt - dvydt;
  }
}

Arrayd WaterObjf::makeInitialParams() {
  Arrayd X(5);
  X.setTo(0.1);
  assert(X.size() == inDims());
  k(X.ptr()) = 1.0;
  return X;
}

void WaterObjf::disp(Arrayd params) {
  assert(params.size() == inDims());
  double *x = params.ptr();

  cout << "Magnetic offset: " << Angle<double>::radians(magOffset(x)).degrees() << " degrees." << endl;
  cout << "k:               " << k(x) << endl;
  cout << "m:               " << m(x) << endl;
  cout << "c:               " << c(x) << endl;
  cout << "alpha:           " << alpha(x) << endl;
}

WaterObjf::WaterObjf(GeographicReference ref,
    Array<Nav> navs,
    FilteredNavs fnavs) :
  _navs(navs),
  _fnavs(fnavs),
  _geoRef(ref) {
  int count = fnavs.times.size();
  assert(count == navs.size());

  Arrayb keep(count);
  int counter = 0;
  for (int i = 0; i < count; i++) {
    double t = fnavs.times[i].seconds();

    // So that there is no drift
    bool isDownwind = cos(fnavs.awa.value(t) - M_PI) > 0;

    // The calibration function is not differentiable when measured speed is 0.
    bool nonNeglibableSpeed = fnavs.watSpeed.value(t) > 0.01;

    bool keepi = isDownwind && nonNeglibableSpeed;
    keep[i] = keepi;
    if (keepi) {
      counter++;
    }
  }
  assert(counter > 5);
  _inds = makeRange(count).slice(keep);
}

void wce001() {
  Array<Nav> navs = getTestNavs(0);

  Array<Duration<double> > T = getLocalTime(navs);
  LineStrip strip = makeNavsLineStrip(T);

  Array<GeographicPosition<double> > pos = getGeoPos(navs);
  GeographicPosition<double> meanPos = mean(pos);
  GeographicReference ref(meanPos);

  FilteredNavs fnavs(navs);
  WaterObjf objf(ref, navs, fnavs);

  Arrayd X = objf.makeInitialParams();

  LevmarSettings settings;

  LevmarState state(X);
  state.minimize(settings, objf);

  Arrayd Xopt = state.getXArray();

  objf.disp(Xopt);
}





} /* namespace sail */
int main() {
  using namespace sail;

  wce001();



  std::cout << "DONE" << std::endl;
  return 0;
}
