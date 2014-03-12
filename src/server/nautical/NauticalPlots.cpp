/*
 *  Created on: 11 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NauticalPlots.h"
#include <server/plot/extra.h>

namespace sail {

MDArray2d getPolarAWAAndWatSpeedData(Array<Nav> navs) {
  int count = navs.size();
  MDArray2d data(count, 2);
  for (int i = 0; i < count; i++) {
    Nav &nav = navs[i];
    double cosa = cos(nav.awa());
    double sina = sin(nav.awa());
    double speed = nav.watSpeed().metersPerSecond();
    data(i, 0) = speed*cosa;
    data(i, 1) = speed*sina;
  }
  return data;
}

void plotPolarAWAAndWatSpeed(Array<Array<Nav> > allNavs) {
  GnuplotExtra plot;
  int count = allNavs.size();
  for (int i = 0; i < count; i++) {
    plot.plot(getPolarAWAAndWatSpeedData(allNavs[i]));
  }
  plot.show();
}

} /* namespace mmm */
