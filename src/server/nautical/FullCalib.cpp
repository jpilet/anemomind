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

  Array<Angle<double> > cleanAngles(Array<Angle<double> > allAngles) {
    Array<Angle<double> > goodAngles = allAngles.slice([](Angle<double> x) {
      return isOrdinary(x.degrees());
    });
    Array<Angle<double> > contAngles = makeContinuousAngles(goodAngles);
    int counter = 0;
    int count = allAngles.size();
    Array<Angle<double> > clean(count);
    for (int i = 0; i < count; i++) {
      if (isOrdinary(allAngles[i].degrees())) {
        clean[i] = contAngles[counter];
        counter++;
      }
    }
    assert(counter == contAngles.size());
    Arrayd degs = clean.map<double>([](Angle<double> x) {return x.degrees();});
    Arrayd cleaned = cleanNumArray(degs);
    assert(!degs.empty());
    assert(!cleaned.empty());
    return cleaned
        .map<Angle<double> >([=](double x) {return Angle<double>::degrees(x);});
  }

  Array<Angle<double> > getAngles(Array<Nav> navs) {
    return navs.map<Angle<double> >([=](const Nav &x) {
      //return x.magHdg();
      //return x.gpsBearing();
      return x.awa();
   });
  }

  Arrayd getTimeSeconds(Array<Nav> navs) {
    TimeStamp first = navs.first().time();
    return navs.map<double>([=](const Nav &x) {return (x.time() - first).seconds();});
  }

  void filtering(Arrayd time, Arrayd angles, int order, double reg) {
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
        for (int i = 0; i < fdydx.size(); i++) {
          std::cout << "fx = " << fxsub[i] << "    fxydx: " << std::abs(fdydxsub[i]) << std::endl;
        }
      }
  }

  void dispAnglesAndFiltered(Array<Nav> allnavs, Spani span) {
    Array<Nav> navs = allnavs.slice(span.minv(), span.maxv());
    Arrayd time = getTimeSeconds(navs);
    Arrayd angles = cleanAngles(getAngles(navs)).map<double>([](Angle<double> x) {return x.degrees();});

    //filtering(time, angles, 2, 5000);
    filtering(time, angles, 1, 1000);
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
