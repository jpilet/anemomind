/*
 *  Created on: 2014-09-06
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/polar/BasicPolar.h>
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <server/nautical/polar/PolarCurves.h>
#include <server/nautical/polar/PolarDensity.h>
#include <server/plot/extra.h>
#include <server/common/string.h>

using namespace sail;

namespace {
  HorizontalMotion<double> calcTW(const Nav &x) {
    double params[TrueWindEstimator::NUM_PARAMS];
    TrueWindEstimator::initializeParameters(params);
    return TrueWindEstimator::computeTrueWind(params, x);
  }

  PolarPoint makePolarPoint(const Nav &x) {
    Angle<double> dir = x.gpsBearing();
    HorizontalMotion<double> tw = calcTW(x);
    return PolarPoint(calcTws(tw),
        calcTwa(tw, dir), x.gpsSpeed());

  }

  Array<PolarPoint> navsToPolarPoints(Array<Nav> navs) {
    return navs.map<PolarPoint>([&](const Nav &x) {return makePolarPoint(x);});
  }

  BasicPolar &getPolar(BasicPolar::TwsHist twsHist, PolarSlice::TwaHist twaHist,
      BasicPolar *cachedPolar, Array<Nav> navs) {
    if (cachedPolar->empty()) {
      *cachedPolar = BasicPolar(twsHist, twaHist, navsToPolarPoints(navs));
    }
    return *cachedPolar;
  }

  PolarSlice::TwaHist makePolarHM(int binCount) {
    return makePolarHistogramMap(binCount,

        // Middle of the first bin maps to an angle of 0 degrees.
        0.5,
        Angle<double>::degrees(0));
  }

  Array<PolarPoint> reduceFromTheSides(Array<PolarPoint> pts, Velocity<double> tws, int count) {
    class OrderByTws {
     public:
      bool operator() (const PolarPoint &a, const PolarPoint &b) {
        return a.tws() < b.tws();
      }
    };
    std::sort(pts.begin(), pts.end(), OrderByTws());
    int removeCount = pts.size() - count;
    double Z = tws.knots();
    for (int i = 0; i < removeCount; i++) {
      if (std::abs(Z - pts.first().tws().knots()) < std::abs(Z - pts.last().tws().knots())) {
        pts = pts.sliceBut(1);
      } else {
        pts = pts.sliceFrom(1);
      }
    }
    return pts;
  }

  Array<Nav> &getCachedNavs(ArgMap &amap, Array<Nav> *navs) {
    if (navs->empty()) {
      *navs = getTestdataNavs(amap);
    }
    return *navs;
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
  amap.registerOption("--plot-for-tws", "Make a plot with a single polar slice close to a [wind-speed-knots] and containing [point-count] points, with bins at [quantile]")
      .setArgCount(3).setUnique();


  double bandwidthKnots = 1.0;
  int boatSpeedSampleCount = 30;
  int twaCount = 30;
  double maxSpeedKnots = 15;
  amap.registerOption("--bandwidth", "Set the bandwidth for the density estimation").setArgCount(1).store(&bandwidthKnots);
  amap.registerOption("--curve", "Make a curve at a [true-windspeed-knots]").setArgCount(2);
  amap.registerOption("--boat-speed-sample-count", "Set number of boat speed samples")
          .setArgCount(1).store(&boatSpeedSampleCount);
  amap.registerOption("--twa-count", "Set number of twa samples for the curve").setArgCount(1).store(&twaCount);

  if (amap.parseAndHelp(argc, argv)) {
    /*BasicPolar::TwsHist twsHist(20,
        Velocity<double>::metersPerSecond(0),
        Velocity<double>::metersPerSecond(20));*/
    BasicPolar::TwsHist twsHist(20,
        Velocity<double>::metersPerSecond(0),
        Velocity<double>::metersPerSecond(20));
    PolarSlice::TwaHist twaHist = makePolarHM(12);

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
      twaHist = makePolarHM(binCount);
    }

    Array<Nav> cachedNavs;
    BasicPolar cachedPolar;

    if (amap.optionProvided("--plot")) {
      double q = amap.optionArgs("--plot")[0]->parseDoubleOrDie();
      CHECK_LT(0.0, q);
      CHECK_LE(q, 1.0);

      getPolar(twsHist, twaHist, &cachedPolar,
          getCachedNavs(amap, &cachedNavs))
        .plot(q);
    }

    if (amap.optionProvided("--plot-for-tws")) {
      Array<ArgMap::Arg*> args = amap.optionArgs("--plot-for-tws");
      Velocity<double> tws = Velocity<double>::knots(args[0]->parseDoubleOrDie());
      CHECK_LE(0.0, tws.knots());
      int count = args[1]->parseIntOrDie();
      CHECK_LE(0, count);
      Array<PolarPoint> pts = reduceFromTheSides(
          navsToPolarPoints(getCachedNavs(amap, &cachedNavs)),
          tws, count);
      double q = args[2]->parseDoubleOrDie();
      CHECK_LT(0.0, q);
      CHECK_LE(q, 1.0);

      BasicPolar(BasicPolar::TwsHist(1, pts.first().tws(), pts.last().tws()),
          twaHist,
          pts)
        .plot(q);
    }

    if (amap.optionProvided("--curve")) {
      PolarDensity density(Velocity<double>::knots(bandwidthKnots),
          navsToPolarPoints(getCachedNavs(amap, &cachedNavs)), true);
      Array<ArgMap::Arg*> args = amap.optionArgs("--curve");
      Velocity<double> tws = Velocity<double>::knots(args[0]->parseDoubleOrDie());
      double q = args[1]->parseDoubleOrDie();
      PolarCurves curve = PolarCurves::fromDensity(density, tws,
          twaCount, Velocity<double>::knots(maxSpeedKnots), boatSpeedSampleCount, q);

      Array<PolarPoint> ptsForVisualization = reduceFromTheSides(
                navsToPolarPoints(getCachedNavs(amap, &cachedNavs)),
                tws, 100);

      GnuplotExtra plot;
      plot.set_style("lines");
      curve.plot(&plot);
      plot.show();
      LOG(INFO) << "Plotting for TWS = " << tws.knots() << " knots or " << tws.metersPerSecond() << " m/s at quantile " << q;
    }

    return 0;
  }
  return -1;
}
