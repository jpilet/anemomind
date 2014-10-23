/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/nautical/TemporalSplit.h>
#include <server/math/ContinuousAngles.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/common/math.h>
#include <server/math/CleanNumArray.h>
#include <server/math/nonlinear/GeneralizedTV.h>


using namespace sail;

namespace {



  Array<Angle<double> > getAngles(Array<Nav> navs) {
    return navs.map<Angle<double> >([=](const Nav &x) {
      //return x.magHdg();
      //return x.gpsBearing();
      return x.awa();
   });
  }

  Array<Duration<double> > getTime(Array<Nav> navs) {
    TimeStamp first = navs.first().time();
    return navs.map<Duration<double> >([=](const Nav &x) {return (x.time() - first);});
  }

  Arrayd toSeconds(Array<Duration<double> > src) {
    return src.map<double>([](Duration<double> x) {return x.seconds();});
  }

  void filtering(Array<Duration<double> > inTime, Array<Angle<double> > inAngles, int order, double reg) {
    Arrayd time = toSeconds(inTime);
    Arrayd angles = inAngles.map<double>([](Angle<double> x) {return x.degrees();});


    //double reg = 5000;
      double spacing = 1.0;
      GeneralizedTV tv;
      UniformSamples filtered =
          tv.filter(time, angles, spacing, order, reg);


      Arrayd fx = filtered.makeCentredX();
      Arrayd fy = filtered.interpolateLinear(fx);
      Arrayd fdydx = filtered.interpolateLinearDerivative(fx);

      {
        GnuplotExtra plot;
        plot.set_style("lines");
        plot.plot_xy(time, angles);
        plot.plot_xy(fx, fy);
        plot.plot_xy(fx, fdydx.map<double>([=](double x) {return 30*x;}));
        plot.set_style("points");
        plot.show();
      }{
        int middle = 5000;
        int marg = 300;
        int from = 4900; //middle - marg;
        int to = 5000; //middle + marg;
        GnuplotExtra plot;
        plot.set_style("lines");
        Arrayd fxsub = fx.slice(from, to);
        Arrayd fdydxsub = fdydx.slice(from, to);
        plot.plot_xy(fxsub, fdydxsub);
        plot.show();
//        for (int i = 0; i < fdydxsub.size(); i++) {
//          std::cout << "fx = " << fxsub[i] << "    fxydx: " << std::abs(fdydxsub[i]) << std::endl;
//        }
      }
  }

  void dispAnglesAndFiltered(Array<Nav> allnavs, Spani span) {
    Array<Nav> navs = allnavs.slice(span.minv(), span.maxv());
    Array<Duration<double> > time = getTime(navs);
    Array<Angle<double> > angles = cleanContinuousAngles(getAngles(navs));

    filtering(time, angles, 2, 5000);
    //filtering(time, angles, 1, 5000);
  }

  void ex0(int index) {
    Array<Nav> navs =
        scanNmeaFolder("/home/jonas/programmering/sailsmart/datasets/psaros33_Banque_Sturdza",
        Nav::debuggingBoatId());
    Array<Spani> spans = recursiveTemporalSplit(navs);
    dispAnglesAndFiltered(navs, spans[index]);
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--ex0", "Run a preconfigured example for race [index]").setArgCount(1);
  if (!amap.parseAndHelp(argc, argv)) {
    return -1;
  }

  if (amap.optionProvided("--ex0")) {
    ex0(amap.optionArgs("--ex0")[0]->parseIntOrDie());
  } else {
    //fullCalib(getTestdataNavs(amap));
    std::cout << "Not yet implemented" << std::endl;
  }
  return 0;
}
