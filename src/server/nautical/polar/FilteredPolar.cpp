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

  Array<LineKM> makeVelMaps(Velocity<double> step) {
    return Array<LineKM>::fill(3, LineKM(0, 1, 0, step.knots()));
  }

  void makeRegPlot(MDArray2d data, MDArray2i inds, double stepSizeKnots, const char *label) {
    assert(data.rows() == 1);
    assert(inds.rows() == 1);
    int count = data.cols();
    assert(count == inds.cols());
    Arrayd X(count);
    Arrayd Yraw(count);
    Arrayd Yfiltered(count);
    for (int i = 0; i < count; i++) {
      X[i] = i;
      Yraw[i] = data(0, i);
      Yfiltered[i] = stepSizeKnots*inds[i];
    }
    GnuplotExtra plot;
    plot.set_style("lines");
    plot.set_ylabel(label);
    plot.plot_xy(X, Yraw);
    plot.plot_xy(X, Yfiltered);
    plot.show();
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
    Velocity<double> stepSize = Velocity<double>::knots(stepSizeKnots);
    Array<Nav> navs = getTestdataNavs(amap);
    Array<PolarPoint> pts = navsToPolarPoints(navs).slice([&](const PolarPoint &x) {return !x.hasNaN();});

    MDArray2d data = makeDataMatrix(pts);
    Array<LineKM> maps = makeVelMaps(stepSize);
    MDArray2i inds = quantFilter(maps, data, lambda);

    if (amap.optionProvided("--plot-x")) {
      const int i = 0;
      makeRegPlot(data.sliceRow(i), inds.sliceRow(i), stepSizeKnots, "X [knots]");
    }
    if (amap.optionProvided("--plot-y")) {
      const int i = 1;
      makeRegPlot(data.sliceRow(i), inds.sliceRow(i), stepSizeKnots, "Y [knots]");
    }
    if (amap.optionProvided("--plot-tws")) {
      const int i = 2;
      makeRegPlot(data.sliceRow(i), inds.sliceRow(i), stepSizeKnots, "TWS [knots]");
    }

    return 0;
  }
  return -1;
}
