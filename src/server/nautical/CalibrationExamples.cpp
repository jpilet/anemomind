/*
 *  Created on: 2014-02-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CalibrationExamples.h"
#include <server/nautical/Nav.h>
#include <server/common/Duration.h>
#include <server/nautical/DataCalib.h>
#include <server/nautical/LocalRace.h>
#include <server/math/nonlinear/GridFitter.h>

namespace sail {



void calibEx001() {
  Array<Nav> allNavs = loadNavsFromText(Nav::AllNavsPath, false);
  Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs,
                                 Duration::minutes(10).getDurationSeconds());
  Array<Nav> navs = splitNavs.first();
  double spaceStep = 500; // metres
  double timeStep = Duration::minutes(10).getDurationSeconds();

  LocalRace localRace(navs, spaceStep, timeStep);

  // The data part of the objective function
  DataCalib calib;
  calib.addBoatData(std::shared_ptr<BoatData>(new BoatData(&localRace, navs)));

  //GridFitter gf;
  //gf.add(std::shared_ptr<GridFit>(new GridFit()));
}

} /* namespace sail */
