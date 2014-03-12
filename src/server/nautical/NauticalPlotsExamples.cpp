/*
 *  Created on: 2014-03-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NauticalPlotsExamples.h"
#include "NauticalPlots.h"

namespace sail {

void npex001() {
  Array<Nav> navs = loadNavsFromText(Nav::AllNavsPath, false);
  Array<Array<Nav> > splitNavs = splitNavsByDuration(
      navs, Duration<>::minutes(10).seconds());

  plotPolarAWAAndWatSpeed(splitNavs);
}

}
