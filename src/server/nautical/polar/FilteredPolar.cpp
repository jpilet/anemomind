/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/plot/extra.h>
#include <server/nautical/TestdataNavs.h>
#include <server/nautical/polar/PolarPointConv.h>
#include <server/math/hmm/QuantFilter.h>
#include <server/math/PolarCoordinates.h>


using namespace sail;

namespace {
  MDArray2d makeDataMatrix(Array<PolarPoint> pts) {
    int count = pts.size();
    MDArray2d dst(3, count);
    for (int i = 0; i < count; i++) {
      PolarPoint &pt = pts[i];
      dst(0, i) = calcPolarX(true, pt.boatSpeed(), pt.twa()).knots();
      dst(1, i) = calcPolarY(true, pt.boatSpeed(), pt.twa()).knots();
      dst(2, i) = pt.tws().knots();
    }
    return dst;
  }

  MDArray2i quantFilterVel(Velocity<double> step, Array<PolarPoint> pts, double reg) {
    MDArray2d data = makeDataMatrix(pts);
    Array<LineKM> maps = Array<LineKM>::fill(3, LineKM(0, 1, 0, step.knots()));
    return quantFilter(maps, data, reg);
  }
}

int main(int argc, const char **argv) {
  double lambda = 1.0;
  double stepSizeKnots = 0.5;

  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--reg", "Set the regularization parameter").setArgCount(1).store(&lambda);
  amap.registerOption("--step", "Set the step size in the quantization").setArgCount(1).store(&stepSizeKnots);
  amap.registerOption("--plot-x", "Plot speed along x-axis in polar plot");
  amap.registerOption("--plot-y", "Plot speed along y-axis in polar plot");
  amap.registerOption("--plot-tws", "Plot true wind speed");

  if (amap.parseAndHelp(argc, argv)) {
    Array<Nav> navs = getTestdataNavs(amap);
    Array<PolarPoint> pts = navsToPolarPoints(navs).slice([&](const PolarPoint &x) {return !x.hasNaN();});

    MDArray2i inds = quantFilterVel(Velocity<double>::knots(stepSizeKnots), pts, lambda);

    return 0;
  }
  return -1;
}
