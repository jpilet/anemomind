/*
 *  Created on: 2014-07-03
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "WaterCalib.h"
#include <server/math/armaadolc.h>
#include <server/math/ADFunction.h>
#include <server/math/GemanMcClure.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/common/SharedPtrUtils.h>
#include <server/math/nonlinear/LevmarSettings.h>
#include <server/plot/extra.h>
#include <server/common/Span.h>
#include <server/common/LineKM.h>

namespace sail {

namespace {
  Array<Nav> getDownwindNavs(Array<Nav> navs) {
    return navs.slice([&](const Nav &n) {
      return cos(n.externalTwa()) < 0;
    });
  }
}

WaterCalib::WaterCalib(const HorizontalMotionParam &param, Velocity<double> sigma, Velocity<double> initR) :
    _param(param), _sigma(sigma), _initR(initR) {}

void WaterCalib::initialize(double *outParams) const {
  wcK(outParams) = SpeedCalib<double>::initK();
  wcM(outParams) = 0;
  wcC(outParams) = 0;
  wcAlpha(outParams) = 0;
  magOffset(outParams) = 0;
  _param.initialize(outParams + 5);
}

Arrayd WaterCalib::makeInitialParams() const {
  Arrayd dst(paramCount());
  initialize(dst.ptr());
  return dst;
}

namespace {
  MDArray2d makeWatCalibCurve(SpeedCalib<double> sc, Span<Velocity<double> > span) {
    const int sampleCount = 1000;
    LineKM map(0, sampleCount-1, WaterCalib::unwrap<double>(span.minv()), WaterCalib::unwrap<double>(span.maxv()));
    MDArray2d dst(sampleCount, 2);
    int counter = 0;
    for (int i = 0; i < sampleCount; i++) {
      double x = map(i);
      if (x > 0) {
        dst(i, 0) = x;
        dst(i, 1) = sc.eval(x);
        counter++;
      }
    }
    return dst.sliceRowsTo(counter);
  }

  MDArray2d makeWatCalibScatter(Array<Nav> navs) {
    int count = navs.size();
    MDArray2d dst(count, 2);
    for (int i = 0; i < count; i++) {
      Nav &n = navs[i];
      double x = WaterCalib::unwrap(n.watSpeed());
      dst(i, 0) = x;
      dst(i, 1) = 0;
    }
    return dst;
  }
}

void WaterCalib::makeWatSpeedCalibPlot(Arrayd params, Array<Nav> navs) const {
  GnuplotExtra plot;
  SpeedCalib<double> sc = makeSpeedCalib(params.ptr());
  Array<Velocity<double> > ws = getWatSpeed(navs);
  Span<Velocity<double> > span = Span<Velocity<double> >(ws).makeWider(Velocity<double>::knots(1.0));


  plot.plot(makeWatCalibScatter(navs), "Samples");

  plot.set_style("lines");
  plot.plot(makeWatCalibCurve(sc, span), "Calibration curve");

  plot.set_xlabel("Raw water speed (m/s)");
  plot.set_ylabel("Calibrated water speed (m/s)");


  plot.show();
}

namespace {
  class WaterCalibObjf : public AutoDiffFunction {
   public:
    WaterCalibObjf(const WaterCalib &calib, Array<Nav> navs) : _calib(calib), _navs(navs) {}
    int inDims() {
      return _calib.paramCount();
    }

    int outDims() {
      return 2*_navs.size();
    }

    void evalAD(adouble *Xin, adouble *Fout);
   private:
    Array<Nav> _navs;
    const WaterCalib &_calib;
  };

  /*
   * Assumtion:
   *
   * Boat-GPS-motion = Water-Current + Boat-Motion-through-water
   *
   *  <=>
   *
   * Boat-GPS-motion - Water-Current - Boat-Motion-through-water = 0   <-- This is how residuals are computed in the lsq problem.
   *
   */
  void WaterCalibObjf::evalAD(adouble *Xin, adouble *Fout) {
    //Arrayad X(inDims(), Xin);
    SpeedCalib<adouble> sc = _calib.makeSpeedCalib<adouble>(Xin);
    int navCount = _navs.size();
    for (int i = 0; i < navCount; i++) {
      adouble *f = Fout + 2*i;
      const Nav &nav = _navs[i];
      HorizontalMotion<adouble> boatWrtWater = _calib.calcBoatMotionRelativeToWater(nav,
            sc, Xin);
      HorizontalMotion<adouble> boatGps = nav.gpsVelocity().cast<adouble>();
      HorizontalMotion<adouble> current = _calib.horizontalMotionParam().get(nav,
          _calib.hmotionParams(Xin));
      HorizontalMotion<adouble> err = boatGps - current - boatWrtWater;
      f[0] = _calib.unwrap(err[0]);
      f[1] = _calib.unwrap(err[1]);
    }
  }
}

WaterCalib::Results WaterCalib::optimize(Array<Nav> allnavs) const {
  return optimize(allnavs, makeInitialParams());
}

WaterCalib::Results WaterCalib::optimizeRandomInit(Array<Nav> navs) const {

}

WaterCalib::Results WaterCalib::optimize(Array<Nav> allnavs, Arrayd initParams) const {
  Array<Nav> navs = getDownwindNavs(allnavs);

  WaterCalibObjf rawObjf(*this, navs);
  GemanMcClureFunction robustObjf(unwrap(_sigma),
      unwrap(_initR), 2,
      makeSharedPtrToStack(rawObjf));

  LevmarSettings settings;
  LevmarState state(initParams);
  state.minimize(settings, robustObjf);
  Arrayd X = state.getXArray();
  return Results(robustObjf.inliers(X.ptr()), X, navs, robustObjf.calcSquaredNorm(X.ptr()));
}


} /* namespace mmm */
