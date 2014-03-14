/*
 *  Created on: 11 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NauticalPlots.h"
#include <server/plot/extra.h>
#include <server/common/PeriodicHist.h>
#include <server/common/string.h>

namespace sail {

bool isReliableNav(Nav nav) {
  return std::abs(nav.awa().radians()) >= 1.0e-6 && std::abs(nav.watSpeed().metersPerSecond()) >= 1.0e-6;
}

Arrayb makeReliableNavMask(Array<Nav> navs) {
  int count = navs.size();
  Arrayb mask(count);
  for (int i = 0; i < count; i++) {
    mask[i] = isReliableNav(navs[i]);
  }
  return mask;
}

MDArray2d getPolarAWAAndWatSpeedData(Array<Nav> navs) {
  int count = navs.size();
  MDArray2d data(count, 2);
  int counter = 0;
  for (int i = 0; i < count; i++) {
    Nav &nav = navs[i];
    Angle<double> angle = nav.awa();
    double speed = nav.watSpeed().metersPerSecond();
    double cosa = cos(angle);
    double sina = sin(angle);
    data(counter, 0) = speed*cosa;
    data(counter, 1) = speed*sina;
    counter++;
  }
  return data.sliceRowsTo(counter);
}

void plotPolarAWAAndWatSpeed(Array<Array<Nav> > allNavs) {
  GnuplotExtra plot;
  plot.set_style("lines");
  int count = allNavs.size();
  for (int i = 0; i < count; i++) {
    Array<Nav> navs = allNavs[i];
    plot.plot(getPolarAWAAndWatSpeedData(navs.slice(makeReliableNavMask(navs))));
  }
  plot.show();
}

PeriodicHist makeAWAHist(Array<Nav> navs, int binCount) {
  PeriodicHist counter(binCount);
  counter.addToAll(1.0e-9);
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    Nav &nav = navs[i];
    Angle<double> angle = nav.awa();
    counter.add(angle, 1.0);
  }
  return counter;
}

void plotHist(PeriodicHist hist) {
  std::cout << EXPR_AND_VAL_AS_STRING(hist) << std::endl;
  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(hist.makePolarPlotData());
  plot.show();
}

PeriodicHist makeAWAWatSpeedSumHist(Array<Nav> navs, int binCount) {
  PeriodicHist sum(binCount);
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    Nav &nav = navs[i];
    Angle<double> angle = nav.awa();
    sum.add(angle, nav.watSpeed().metersPerSecond());
  }
  return sum;
}

PeriodicHist makeAWAAWSSumHist(Array<Nav> navs, int binCount) {
  PeriodicHist sum(binCount);
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    Nav &nav = navs[i];
    Angle<double> angle = nav.awa();
    sum.add(angle, nav.aws().metersPerSecond());
  }
  return sum;
}

} /* namespace mmm */
