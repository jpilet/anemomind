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
      if (isOrdinary(allAngles[i])) {
        clean[i] = contAngles[counter];
        counter++;
      }
    }
    assert(counter == contAngles.size());
    return makeOrdinary(clean.map<double>([](Angle<double> x) {return x.degrees();}))
        .map<Angle<double> >([](double x) {return Angle<double>::degrees(x);});
  }

  Array<Angle<double> > getAwa(Array<Nav> navs) {
    return navs.map<Angle<double> >([=](const Nav &x) {return x.awa();});
  }

  Arrayd getTimeSeconds(Array<Nav> navs) {
    TimeStamp first = navs.first().time();
    return navs.map<double>([=](const Nav &x) {return (x.time() - first).seconds();});
  }

  void dispAwa(Array<Nav> allnavs, Spani span) {
    Array<Nav> navs = allnavs.slice(span.minv(), span.maxv());

    GnuplotExtra plot;
    plot.set_style("lines");
    Arrayd time = getTimeSeconds(navs);
    Arrayd angles = makeContinuousAngles(getAwa(navs)).map<double>([](Angle<double> x) {return x.degrees();});
    Spand timesp(time);
    Spand anglesp(angles);
    std::cout << EXPR_AND_VAL_AS_STRING(timesp) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(anglesp) << std::endl;
    plot.plot_xy(time, angles);
    //plot.plot_xy(time, time);
    plot.show();
  }

  void fullCalib(Array<Nav> navs) {
    Array<Spani> spans = recursiveTemporalSplit(navs);
    dispAwa(navs, spans[1]);
  }

  void ex0() {
    Array<Nav> navs =
        scanNmeaFolder("/home/jonas/programmering/sailsmart/datasets/psaros33_Banque_Sturdza",
        Nav::debuggingBoatId());
    fullCalib(navs);
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--ex0", "Run a preconfigured example");
  if (!amap.parseAndHelp(argc, argv)) {
    return -1;
  }

  if (amap.optionProvided("--ex0")) {
    ex0();
  } else {
    fullCalib(getTestdataNavs(amap));
  }
  return 0;
}
