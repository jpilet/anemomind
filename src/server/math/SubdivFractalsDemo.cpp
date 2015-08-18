/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <random>
#include <server/plot/extra.h>
#include <server/math/SubdivFractals.h>
#include <server/common/ArgMap.h>
#include <server/common/LineKM.h>
#include <limits>

using namespace sail;
using namespace sail::SubdivFractals;

typedef Fractal<1> Frac1d;
typedef Fractal<2> Frac2d;

template <int Dim>
Array<Vertex> initCtrl(int classCount, double maxv) {
  Array<Vertex> dst(Fractal<2>::ctrlCount);
  for (int i = 0; i < Fractal<Dim>::ctrlCount; i++) {
    dst[i] = Vertex(maxv*(i % 2), i % classCount);
  }
  return dst;
}

void plotFractal1d(const Frac1d &f, double minx, double maxx) {
  int sampleCount = 4000;
  Arrayd X(sampleCount);
  Arrayd Y(sampleCount);
  LineKM map(0, sampleCount-1, minx, maxx);
  for (int i = 0; i < sampleCount; i++) {
    double x = map(i);
    X[i] = x;
    double xcoord[Frac1d::spaceDimension];
    for (int j = 0; j < Frac1d::spaceDimension; j++) {
      xcoord[j] = x;
    }
    Y[i] = f.eval(xcoord);
  }

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot_xy(X, Y);
  plot.show();
}

double toX(double x, double y) {
  return x + 0.5*y;
}

double toY(double x, double y) {
  return 0.5*sqrt(3.0)*y;
}

void plotSlice(LineKM xmap, LineKM ymap, Frac2d f, GnuplotExtra *plot) {
  int sampleCount = 4000;
  LineKM map01(0, sampleCount-1, 0, 1);
  Arrayd X(sampleCount), Y(sampleCount), Z(sampleCount);
  for (int i = 0; i < sampleCount; i++) {
    double x0 = xmap(map01(i));
    double y0 = ymap(map01(i));
    double coords[2] = {x0, y0};
    Z[i] = f.eval(coords);
    X[i] = toX(x0, y0);
    Y[i] = toY(x0, y0);
  }
  plot->plot_xyz(X, Y, Z);
}

void plotFractal2d(const Frac2d &f) {
  int sparseCount = 9;
  LineKM sparseMap(0, sparseCount-1, 0.0, 1.0);

  GnuplotExtra plot;
  plot.set_style("lines");
  for (int i = 0; i < sparseCount; i++) {
    double sp = sparseMap(i);
    plotSlice(LineKM(0, 1, sp, sp), LineKM(0, 1, 0, 1.0 - sp), f, &plot);
    plotSlice(LineKM(0, 1, 0, 1.0 - sp), LineKM(0, 1, sp, sp), f, &plot);
    plotSlice(LineKM(0, 1, sp, 0), LineKM(0, 1, 0, sp), f, &plot);
  }
  plot.show();
}


int main(int argc, const char **argv) {

  int classCount = 5;
  double minx = 0.0;
  double maxx = 1.0;
  int depth = 12;
  int seed = 0;
  double maxv = std::numeric_limits<double>::infinity();
  double maxSlope = 1.0;

  ArgMap amap;
  amap.registerOption("--minx",
      "Minimum x value for which to plot (default 0").store(&minx);
  amap.registerOption("--maxx",
      "Maximum x value for which to plot (default 1").store(&maxx);
  amap.registerOption("--depth",
      "How deeply we evaluate the fractal (default 12)")
      .store(&depth);
  amap.registerOption("--class-count",
      "How many classes there are (default 5)")
    .store(&classCount);
  amap.registerOption("--seed", "Seed to the random number generator (default 0)")
      .store(&seed);
  amap.registerOption("--max", "Indicative value on the maximum absolute value of the signal (default infinity)")
    .store(&maxv);
  amap.registerOption("--max-slope", "Indicative value on the maximum absolute value of the local derivative (default 1.0)")
      .store(&maxSlope);
  amap.registerOption("--generate", "Generate code for this fractal.");
  amap.registerOption("--2d", "Plot in a 2d space");
  if (amap.parse(argc, argv) == ArgMap::Error) {
    return -1;
  } else if (amap.helpAsked()) {
    return 0;
  }

  std::default_random_engine e(seed);

  MDArray<Rule::Ptr, 2> rules;
  double r = 0;
  MaxSlope slope(maxv, maxSlope);
  rules = makeRandomBoundedRules(classCount, slope, e);



  if (amap.optionProvided("--2d")) {
    Array<Vertex> ctrl = initCtrl<2>(classCount, r);
    Fractal<2> f2(rules, ctrl, depth);
    plotFractal2d(f2);
  } else {
    Array<Vertex> ctrl = initCtrl<2>(classCount, r);
    Fractal<1> f(rules, ctrl, depth);
    plotFractal1d(f, minx, maxx);
    if (amap.optionProvided("--generate")) {
      f.generateCode("rules");
    }
  }



  return 0;
}


