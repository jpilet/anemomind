/*
 *  Created on: 2014-03-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/Nav.h>
#include <server/plot/extra.h>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>
#include <server/nautical/FilteredNavs.h>
#include <server/math/nonlinear/SignalFit.h>
#include <server/common/split.h>
#include <server/math/nonlinear/LevmarSettings.h>

using namespace sail;




void fnex001() { // Various plots
  Array<Nav> navs = getTestNavs(0).slice(1000, 2000);

  Arrayd X = getLocalTime(navs).map<double>([&](Duration<double> t) {return t.seconds();});


  //Arrayd Y = getAwa(navs).map<double>([&] (Angle<double> awa) {return awa.degrees();});
  //Arrayd Y = getAws(navs).map<double>([&] (Velocity<double> x) {return x.metersPerSecond();});

  // RELIABLE:
  //Arrayd Y = getMagHdg(navs).map<double>([&] (Angle<double> x) {return x.degrees();});
  //Arrayd Y = getGpsBearing(navs).map<double>([&] (Angle<double> x) {return x.degrees();});
  Arrayd Y = getGpsSpeed(navs).map<double>([&] (Velocity<double> x) {return x.metersPerSecond();});
  Arrayd Y2 = getWatSpeed(navs).map<double>([&] (Velocity<double> x) {return x.metersPerSecond();});

  std::cout << EXPR_AND_VAL_AS_STRING(X) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(Y) << std::endl;

  GnuplotExtra plot;
  plot.plot_xy(X, Y);
  plot.plot_xy(X, Y2);
  plot.show();
}

void fnex002() { // Filter a signal
  Array<Nav> navs = getTestNavs(0).sliceTo(1000); //.slice(1000, 1500);

  Arrayd X = getLocalTime(navs).map<double>([&](Duration<double> t) {return t.seconds();});

  Array<Velocity<double> > Y = getAws(navs);
  Arrayb rel = identifyReliableAws(Y);
  Arrayd Yd = Y.map<double>([&](Velocity<double> x) {return x.knots();});
  std::cout << EXPR_AND_VAL_AS_STRING((countTrue(rel))) << std::endl;

  LineStrip strip(Span(X).expand(0.1), 1.0);
  Arrayd Xlines = strip.getGridVertexCoords1d();
  LevmarSettings settings;
  Arrayd Ylines = fitLineStripAutoTune(strip, makeRange(2, 1), X.slice(rel), Yd.slice(rel), makeRandomSplits(9, countTrue(rel)), settings).vertices;

  Arrayb unrel = neg(rel);

  GnuplotExtra plot;
  plot.plot_xy(X.slice(rel), Yd.slice(rel));
  plot.plot_xy(X.slice(unrel), Yd.slice(unrel));
  plot.set_style("lines");
  plot.plot_xy(Xlines, Ylines);
  plot.show();

}


void fnex003() { // Plot true vel vs wat speed
  Array<Nav> navs = getTestNavs(0).slice(1000, 2000);

  Arrayd Y = getGpsSpeed(navs).map<double>([&] (Velocity<double> x) {return x.metersPerSecond();});
  Arrayd Y2 = getWatSpeed(navs).map<double>([&] (Velocity<double> x) {return x.metersPerSecond();});

  GnuplotExtra plot;
  plot.plot_xy(Y2, Y);
  plot.show();
}

void fnex004() { // Filter AWA
  Array<Nav> navs = getTestNavs(0).slice(1000, 2000);
  Arrayd X = getLocalTime(navs).map<double>([&](Duration<double> t) {return t.seconds();});
  Arrayb rel = identifyReliableAwa(getAwa(navs));

  std::cout << EXPR_AND_VAL_AS_STRING(countTrue(rel)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(rel.size()) << std::endl;
}

int main() {
  fnex004();

  return -1;
}


