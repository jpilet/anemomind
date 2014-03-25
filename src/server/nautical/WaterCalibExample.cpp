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
#include <server/common/OptimResults.h>
#include <server/common/Uniform.h>
#include <server/common/ScopedLog.h>
#include <server/common/split.h>
#include <server/common/ArrayIO.h>
#include <server/common/Statistics.h>

namespace sail {

class WaterObjf : public AutoDiffFunction {
 public:
  WaterObjf(GeographicReference ref, Array<Nav> navs,
      FilteredNavs fnavs, bool withCurrent_);

  // [Magnetic offset] + [SpeedCalib]
  int inDims() {return 1 + 4 + (_withCurrent? 2 : 0);}
  int outDims() {return 1 + 2*_inds.size();}

  void evalAD(adouble *Xin, adouble *Fout);
  void evalADDeriv(adouble *Xin, adouble *Fout);
  void evalADAbs(adouble *Xin, adouble *Fout);

  Arrayd makeInitialDefaultParams();
  Arrayd makeRandomParams();
  template <typename T> T &magOffset(T *x) {return x[0];}
  template <typename T> T &k(T *x) {return x[1];}
  template <typename T> T &m(T *x) {return x[2];}
  template <typename T> T &c(T *x) {return x[3];}
  template <typename T> T &alpha(T *x) {return x[4];}
  template <typename T> T &currentX(T *x) {assert(_withCurrent); return x[5];}
  template <typename T> T &currentY(T *x) {assert(_withCurrent); return x[6];}


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

  MinimizationResults minimizeRandomInits(int initCount, LevmarSettings s);
  MinimizationResults minimizeWithInit(Arrayd X, LevmarSettings settings);

  Arrayi inds() {return _inds;}
  void setInds(Arrayi I) {_inds = I;}

  MDArray2d optimizeForSplits(Array<Arrayb> includePerSplit, int initCount, LevmarSettings s);
  MDArray2d optimize2Fold(int initCount, LevmarSettings s);
  MDArray2d optimizeNFold(int N, int initCount, LevmarSettings s);

  Array<Velocity<double> > getWatSpeeds();

  bool withCurrent() const {return _withCurrent;}
 private:
  bool _withCurrent;
  Arrayi _inds;

  GeographicReference _geoRef;
  Array<Nav> _navs;
  FilteredNavs _fnavs;

};

void WaterObjf::evalAD(adouble *Xin, adouble *Fout) {
  evalADAbs(Xin, Fout);
  SpeedCalib<adouble> calib = makeSpeedCalib(Xin);
  Fout[outDims()-1] = calib.ambiguityPenalty();
}


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

    if (_withCurrent) {
      rx += currentX(Xin);
      ry += currentY(Xin);
    }

    f[0] = vx - rx;
    f[1] = vy - ry;
  }
}




Arrayd WaterObjf::makeInitialDefaultParams() {
  Arrayd X(inDims());
  X.setTo(0.01);
  magOffset(X.ptr()) = 0.5*M_PI;
  k(X.ptr()) = SpeedCalib<double>::initK();
  alpha(X.ptr()) = 1.0;
  return X;
}

Arrayd WaterObjf::makeRandomParams() {
  Arrayd X(inDims());
  Uniform rng(0.001, 2.0);
  for (int i = 0; i < X.size(); i++) {
    X[i] = rng.gen();
  }
  return X;
}

MinimizationResults WaterObjf::minimizeRandomInits(int initCount, LevmarSettings s) {
  MinimizationResults x = minimizeWithInit(makeInitialDefaultParams(), s);
  for (int i = 0; i < initCount; i++) {
    ENTERSCOPE(stringFormat("Minimize for random parameters %d/%d", i+1, initCount));
    x = std::min(x, minimizeWithInit(makeRandomParams(), s));
  }
  return x;
}

MinimizationResults WaterObjf::minimizeWithInit(Arrayd X, LevmarSettings settings) {
  LevmarState state(X);
  {
    WithScopedLogDepth wd(0);
    state.minimize(settings, *this);
  }
  Arrayd Xopt = state.getXArray();
  return MinimizationResults(this->calcSquaredNorm(Xopt.ptr()), Xopt);
}

MDArray2d WaterObjf::optimizeForSplits(Array<Arrayb> includePerSplit, int initCount, LevmarSettings s) {
  Arrayi allInds = inds();
  int splitCount = includePerSplit.size();
  MDArray2d results(inDims(), splitCount);
  for (int i = 0; i < splitCount; i++) {
    ENTERSCOPE(stringFormat("Optimize for split %d/%d", i+1, splitCount));

    WithScopedLogDepth wd(0);
    setInds(allInds.slice(includePerSplit[i]));
    MinimizationResults opt = minimizeRandomInits(initCount, s);
    MDArray2d(opt.X()).copyToSafe(results.sliceCol(i));
  }
  setInds(allInds);
  return results;
}

MDArray2d WaterObjf::optimize2Fold(int initCount, LevmarSettings s) {
  return optimizeNFold(2, initCount, s);
}


MDArray2d WaterObjf::optimizeNFold(int N, int initCount, LevmarSettings s) {
  Array<Arrayb> folds(N);
  for (int i = 0; i < N; i++) {
    folds[i] = makeFoldSplit(_inds.size(), N, i);
  }
  return optimizeForSplits(folds, initCount, s);
}

Array<Velocity<double> > WaterObjf::getWatSpeeds() {
  int count = _inds.size();
  Array<Velocity<double> > ws(count);
  for (int I = 0; I < count; I++) {
    int i = _inds[I];
    double t = _fnavs.times[i].seconds();
    ws[I] = Velocity<double>::metersPerSecond(_fnavs.watSpeed.value(t));
  }
  return ws;
}

void WaterObjf::disp(Arrayd params) {
  assert(params.size() == inDims());
  double *x = params.ptr();

  cout << "Number of measurements: " << _inds.size() << endl;
  cout << "Magnetic offset: " << Angle<double>::radians(magOffset(x)).degrees() << " degrees." << endl;

  SpeedCalib<double> calib = makeSpeedCalib(params.ptr());
  std::cout << EXPR_AND_VAL_AS_STRING(calib.scaleCoef()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(calib.offsetCoef()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(calib.nonlinCoef()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(calib.decayCoef()) << std::endl;

  if (_withCurrent) {
    std::cout << EXPR_AND_VAL_AS_STRING(currentX(x)) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(currentY(x)) << std::endl;
  }
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
    FilteredNavs fnavs,
    bool withCurrent_) :
  _navs(navs),
  _fnavs(fnavs),
  _geoRef(ref),
  _withCurrent(withCurrent_) {
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


namespace {
  MDArray2d makeKnotPlot(Array<Velocity<double> > ws) {
    int count = ws.size();
    MDArray2d XY(count, 2);
    XY.setAll(0.0);
    for (int i = 0; i < count; i++) {
      XY(i, 0) = ws[i].knots();
    }
    return XY;
  }
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
  WaterObjf objf(ref, navs, fnavs, false);

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


/*
 * Estimate parameters using all measurements and random initialization.
 */
void wce002() { // WORKS WELL
  Array<Nav> navs = getTestNavs(0);

  Array<Duration<double> > T = getLocalTime(navs);
  LineStrip strip = makeNavsLineStrip(T);

  Array<GeographicPosition<double> > pos = getGeoPos(navs);
  GeographicPosition<double> meanPos = mean(pos);
  GeographicReference ref(meanPos);

  FilteredNavs fnavs(navs);
  WaterObjf objf(ref, navs, fnavs, false);

  LevmarSettings settings;

  MinimizationResults results = objf.minimizeRandomInits(30, settings);

  Arrayd Xopt = results.X();
  objf.disp(Xopt);

  MDArray2d pdata = objf.makeWaterSpeedCalibPlotData(Xopt);
  Arrayd mes = pdata.sliceCol(0).getStorage().sliceTo(pdata.rows());

  Array<Velocity<double> > ws = objf.getWatSpeeds();
  Statistics wsstats;
  for (int i = 0; i < ws.size(); i++) {
    wsstats.add(ws[i].knots());
  }
  std::cout << EXPR_AND_VAL_AS_STRING(wsstats) << std::endl;

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(pdata);
  plot.plot_xy(mes, mes);
  plot.set_style("points");
  plot.plot(makeKnotPlot(ws));
  plot.show();
}

void wce003() { // A different result
  Array<Nav> navs = getTestNavs(0);

  Array<Duration<double> > T = getLocalTime(navs);
  LineStrip strip = makeNavsLineStrip(T);

  Array<GeographicPosition<double> > pos = getGeoPos(navs);
  GeographicPosition<double> meanPos = mean(pos);
  GeographicReference ref(meanPos);

  FilteredNavs fnavs(navs);
  WaterObjf objf(ref, navs, fnavs, true);

  LevmarSettings settings;

  MinimizationResults results = objf.minimizeRandomInits(30, settings);

  Arrayd Xopt = results.X();
  objf.disp(Xopt);

  MDArray2d pdata = objf.makeWaterSpeedCalibPlotData(Xopt);
  Arrayd mes = pdata.sliceCol(0).getStorage().sliceTo(pdata.rows());

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(pdata);
  plot.plot_xy(mes, mes);
  plot.show();
}

void wce004() { // Display random splits
  Array<Arrayb> X = makeRandomlySlidedFolds(3, 30);
  std::cout << EXPR_AND_VAL_AS_STRING(X) << std::endl;
}





void wce005() { // With cross validation: Randomly slided 2-folds
  bool compute = false;
  int navIndex = 0;

  Array<Array<Nav> > allNavs = getAllTestNavs();
  for (int navIndex = 0; navIndex < allNavs.size(); navIndex++) {
    ENTERSCOPE(stringFormat("===== Processing navs %d/%d", navIndex+1, allNavs.size()));

    Array<Nav> navs = allNavs[navIndex];

    Array<Duration<double> > T = getLocalTime(navs);
    LineStrip strip = makeNavsLineStrip(T);

    Array<GeographicPosition<double> > pos = getGeoPos(navs);
    GeographicPosition<double> meanPos = mean(pos);
    GeographicReference ref(meanPos);

    FilteredNavs fnavs(navs);
    WaterObjf objf(ref, navs, fnavs, false);

    LevmarSettings settings;


    std::string filename = stringFormat("wce005_navindex%02d_withexp%d_withcurrent%d",
        navIndex,
        SpeedCalib<double>::withExp,
        objf.withCurrent());

    if (compute) {
      Array<Arrayb> splits = makeRandomlySlidedFolds(6, objf.inds().size());
      MDArray2d params = objf.optimizeForSplits(splits, 30, settings);
      std::cout << EXPR_AND_VAL_AS_STRING(params) << std::endl;
      saveMatrix(filename, params);
    } else {
      MDArray2d data = loadMatrixText<double>(filename);

      // Magnetic heading
      Statistics maghdg;
      for (int i = 0; i < data.cols(); i++) {
        maghdg.add(Angle<double>::radians(data(0, i)).degrees());
      }
      std::cout << EXPR_AND_VAL_AS_STRING(maghdg) << std::endl;

      // Speed calibration
      const int knotSampleCount = 6;
      LineKM knotMap(0, knotSampleCount-1, 1, 6);
      Statistics perKnot[knotSampleCount];
      for (int i = 0; i < data.cols(); i++) {
        SpeedCalib<double> calib = objf.makeSpeedCalib(data.sliceCol(i).ptr());
        for (int j = 0; j < knotSampleCount; j++) {
          Velocity<double> measVel = Velocity<double>::knots(knotMap(j));
          Velocity<double> trueVel = Velocity<double>::metersPerSecond(calib.eval(measVel.metersPerSecond()));
          perKnot[j].add(trueVel.knots());
        }
      }
      for (int i = 0; i < knotSampleCount; i++) {
        cout << knotMap(i) << " knots map to " << perKnot[i] << " knots" << endl;
      }

      // Current
      if (objf.withCurrent()) {
        Statistics curX, curY;
        for (int i = 0; i < data.cols(); i++) {
          double *x = data.sliceCol(i).ptr();
          curX.add(objf.currentX(x));
          curY.add(objf.currentY(x));
        }
        std::cout << EXPR_AND_VAL_AS_STRING(curX) << std::endl;
        std::cout << EXPR_AND_VAL_AS_STRING(curY) << std::endl;
      }
    }
  }
}


} /* namespace sail */
int main() {
  using namespace sail;

  wce002();

  std::cout << "DONE" << std::endl;
  return 0;
}
