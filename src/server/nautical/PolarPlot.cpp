/*
 *  Created on: 2014-09-06
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/BasicPolar.h>
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>

using namespace sail;

namespace {
  HorizontalMotion<double> calcTW(const Nav &x) {
    double params[TrueWindEstimator::NUM_PARAMS];
    TrueWindEstimator::initializeParameters(params);
    return TrueWindEstimator::computeTrueWind(params, x);
  }

  PolarPoint makePP(const Nav &x) {
    Angle<double> dir = x.gpsBearing();
    HorizontalMotion<double> tw = calcTW(x);
    return PolarPoint(calcTws(tw),
        calcTwa(tw, dir), x.gpsSpeed());

  }

  Array<PolarPoint> navsToPolarPoints(Array<Nav> navs) {
    return navs.map<PolarPoint>([&](const Nav &x) {return makePP(x);});
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--tws-hist",
      "Specify [bin-count] [min-speed-knots] [max-speed-knots] for the true wind speed.")
      .setArgCount(3).setUnique();
  amap.registerOption("--twa-hist",
      "Specify [bin-count] for the true wind angle").setArgCount(2).setUnique();
  amap.registerOption("--plot", "Make a plot, specifying [quantile-frac]").setArgCount(1).setUnique();
  if (amap.parseAndHelp(argc, argv)) {
    BasicPolar::TwsHist twsHist(20,
        Velocity<double>::metersPerSecond(0),
        Velocity<double>::metersPerSecond(20));
    PolarSlice::TwaHist twaHist = makePolarHistogramMap(12);

    if (amap.optionProvided("--tws-hist")) {
      Array<ArgMap::Arg*> args = amap.optionArgs("--tws-hist");
      int binCount = args[0]->parseIntOrDie();
      CHECK_LT(0, binCount);
      double fromTwsSpeed = args[1]->parseDoubleOrDie();
      double toTwsSpeed = args[2]->parseDoubleOrDie();
      CHECK_LT(fromTwsSpeed, toTwsSpeed);
      twsHist = BasicPolar::TwsHist(binCount,
          Velocity<double>::knots(fromTwsSpeed),
          Velocity<double>::knots(toTwsSpeed));
    }

    if (amap.optionProvided("--twa-hist")) {
      int binCount = amap.optionArgs("--twa-hist")[0]->parseIntOrDie();
      CHECK_LT(0, binCount);
      twaHist = makePolarHistogramMap(binCount);
    }

    Array<Nav> navs = getTestdataNavs(amap);
    BasicPolar polar(twsHist, twaHist, navsToPolarPoints(navs));

    if (amap.optionProvided("--plot")) {
      double q = amap.optionArgs("--plot")[0]->parseDoubleOrDie();
      CHECK_LT(0.0, q);
      CHECK_LE(q, 1.0);
      polar.plot(q);
    }

    return 0;
  }
  return -1;
}
