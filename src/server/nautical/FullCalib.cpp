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
    return navs.map<Angle<double> >([=](const Nav &x) {return x.magHdg();});
  }

  Arrayd getTimeSeconds(Array<Nav> navs) {
    TimeStamp first = navs.first().time();
    return navs.map<double>([=](const Nav &x) {return (x.time() - first).seconds();});
  }

  void dispAnglesAndFiltered(Array<Nav> allnavs, Spani span) {
    Array<Nav> navs = allnavs.slice(span.minv(), span.maxv());

    GnuplotExtra plot;
    plot.set_style("lines");
    Arrayd time = getTimeSeconds(navs);
    Arrayd angles = cleanAngles(getAngles(navs)).map<double>([](Angle<double> x) {return x.degrees();});

    plot.plot_xy(time, angles);
    plot.show();
  }

  void ex0() {
    Array<Nav> navs =
        scanNmeaFolder("/home/jonas/programmering/sailsmart/datasets/psaros33_Banque_Sturdza",
        Nav::debuggingBoatId());
    Array<Spani> spans = recursiveTemporalSplit(navs);
    dispAnglesAndFiltered(navs, spans[4]);
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
    //fullCalib(getTestdataNavs(amap));
    std::cout << "Not yet implemented" << std::endl;
  }
  return 0;
}
