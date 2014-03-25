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
#include <server/plot/extra.h>


namespace sail {

class WaterObjf : public AutoDiffFunction {
 public:
  WaterObjf(GeographicReference ref, Array<Nav> navs,
      FilteredNavs fnavs);

  // [Magnetic offset] + [SpeedCalib]
  int inDims() {return 1 + 4;}
  int outDims() {return 2*_inds.size();}

  void evalAD(adouble *Xin, adouble *Fout) {evalADAbs(Xin, Fout);}
  void evalADDeriv(adouble *Xin, adouble *Fout);
  void evalADAbs(adouble *Xin, adouble *Fout);

  Arrayd makeInitialDefaultParams();
  template <typename T> T &magOffset(T *x) {return x[0];}
  template <typename T> T &k(T *x) {return x[1];}
  template <typename T> T &m(T *x) {return x[2];}
  template <typename T> T &c(T *x) {return x[3];}
  template <typename T> T &alpha(T *x) {return x[4];}

  template <typename T>
  SpeedCalib<T> makeSpeedCalib(T *Xin) {
    return SpeedCalib<T>(k(Xin), m(Xin), c(Xin), alpha(Xin));
  }

  template <typename T>
  SpeedCalib<T> makeSpeedCalib(Array<T> Xin) {
    return makeSpeedCalib<T>(Xin.ptr());
  }


  void disp(Arrayd params);
  MDArray2d makeWaterSpeedCalibPlotData(Arrayd params);
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



void WaterObjf::evalADDeriv(adouble *Xin, adouble *Fout) {
  int count = _inds.size();
  SpeedCalib<adouble> calib = makeSpeedCalib(Xin);
  for (int I = 0; I < count; I++) {
    int i = _inds[I];

    double t = _fnavs.times[i].seconds();
    adouble *f = Fout + 2*I;

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

void WaterObjf::evalADAbs(adouble *Xin, adouble *Fout) {
  int count = _inds.size();
  SpeedCalib<adouble> calib = makeSpeedCalib(Xin);
  for (int I = 0; I < count; I++) {
    int i = _inds[I];

    double t = _fnavs.times[i].seconds();
    adouble *f = Fout + 2*I;

    adouble measuredWatSpeed = _fnavs.watSpeed.value(t);

    adouble r = calib.eval(measuredWatSpeed);
    adouble av = _fnavs.magHdg.value(t) + magOffset(Xin);
    adouble rx = r*sin(av);
    adouble ry = r*cos(av);

    // Derivatives of gps speed. Constant w.r.t. parameters
    double b = _fnavs.gpsBearing.value(t);
    double v = _fnavs.gpsSpeed.value(t);
    double vx = v*sin(b);
    double vy = v*cos(b);

    f[0] = vx - rx;
    f[1] = vy - ry;
  }
}




Arrayd WaterObjf::makeInitialDefaultParams() {
  Arrayd X(5);
  assert(X.size() == inDims());
  X.setTo(0.1);
  magOffset(X.ptr()) = 0.5*M_PI;
  k(X.ptr()) = 1.0;
  return X;
}

void WaterObjf::disp(Arrayd params) {
  assert(params.size() == inDims());
  double *x = params.ptr();

  cout << "Number of measurements: " << _inds.size() << endl;
  cout << "Magnetic offset: " << Angle<double>::radians(magOffset(x)).degrees() << " degrees." << endl;
  cout << "k:               " << k(x) << endl;
  cout << "m:               " << m(x) << endl;
  cout << "c:               " << c(x) << endl;
  cout << "alpha:           " << alpha(x) << endl;
}

MDArray2d WaterObjf::makeWaterSpeedCalibPlotData(Arrayd params) {
  assert(params.size() == inDims());
  double *x = params.ptr();

  SpeedCalib<double> calib = makeSpeedCalib(params);

  int count = 1000;
  MDArray2d data(count, 2);
  Velocity<double> maxvel = Velocity<double>::knots(20.0);
  LineKM velmap(0, count-1, 0.001, maxvel.metersPerSecond());
  for (int i = 0; i < count; i++) {
    data(i, 0) = Velocity<double>::metersPerSecond(velmap(i)).knots();
    data(i, 1) = Velocity<double>::metersPerSecond(calib.eval(velmap(i))).knots();
  }
  return data;
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
  navs = navs.sliceTo(navs.middle());

  Array<Duration<double> > T = getLocalTime(navs);
  LineStrip strip = makeNavsLineStrip(T);

  Array<GeographicPosition<double> > pos = getGeoPos(navs);
  GeographicPosition<double> meanPos = mean(pos);
  GeographicReference ref(meanPos);

  FilteredNavs fnavs(navs);
  WaterObjf objf(ref, navs, fnavs);

  Arrayd X = objf.makeInitialDefaultParams();

  LevmarSettings settings;

  LevmarState state(X);
  state.minimize(settings, objf);

  Arrayd Xopt = state.getXArray();

  objf.disp(Xopt);

  MDArray2d pdata = objf.makeWaterSpeedCalibPlotData(Xopt);
  Arrayd mes = pdata.sliceCol(0).getStorage().sliceTo(pdata.rows());

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(pdata);
  plot.plot_xy(mes, mes);
  plot.show();
}





} /* namespace sail */
int main() {
  using namespace sail;

  wce001();



  std::cout << "DONE" << std::endl;
  return 0;
}
