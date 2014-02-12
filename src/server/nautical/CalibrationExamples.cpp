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
#include <armadillo>
#include <server/common/string.h>

namespace sail {



void calibEx001() {
  Array<Nav> allNavs = loadNavsFromText(Nav::AllNavsPath, false);
  Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs,
                                 Duration::minutes(10).getDurationSeconds());
  Array<Nav> navs = splitNavs.first();
  double spaceStep = 500; // metres
  double timeStep = Duration::minutes(10).getDurationSeconds();

  LocalRace race(navs, spaceStep, timeStep);
  Grid3d wgrid = race.getWindGrid();
  Grid3d cgrid = race.getCurrentGrid();

  std::shared_ptr<BoatData> boatData(new BoatData(&race, navs));

  DataCalib calib;
  calib.addBoatData(boatData);

  WindData windData(calib);
  CurrentData currentData(calib);

  arma::sp_mat Pwind = kronWithSpEye(calib.makeP(wgrid), 2);
  arma::sp_mat windRegSpace = kronWithSpEye(vcat(wgrid.makeFirstOrderReg(0),
                                   wgrid.makeFirstOrderReg(1)), 2);
  arma::sp_mat windRegTime = kronWithSpEye(wgrid.makeFirstOrderReg(2), 2);

  arma::sp_mat Pcurrent = kronWithSpEye(calib.makeP(cgrid), 2);
  arma::sp_mat currentRegSpace = kronWithSpEye(vcat(cgrid.makeFirstOrderReg(0),
                                   cgrid.makeFirstOrderReg(1)), 2);
  arma::sp_mat currentRegTime = kronWithSpEye(cgrid.makeFirstOrderReg(2), 2);


  const double initialRegWeight = 0.1;

  const int splitCount = 4;
  Array<Arrayb> windSplits = makeRandomSplits(4, calib.windDataCount());
  Array<Arrayb> currentSplits = makeRandomSplits(4, calib.currentDataCount());

  double windCurrentBalance = 0.5;
  std::shared_ptr<GridFit> windTerm(new GridFit(Pwind, &windData,
      Array<arma::sp_mat>::args(windRegSpace, windRegTime), windSplits,
      Arrayd::args(initialRegWeight, initialRegWeight),
      Array<std::string>::args("Wind-space", "Wind-time"),
      windCurrentBalance));
  std::shared_ptr<GridFit> currentTerm(new GridFit(Pcurrent, &currentData,
      Array<arma::sp_mat>::args(currentRegSpace, currentRegTime), currentSplits,
      Arrayd::args(initialRegWeight, initialRegWeight),
      Array<std::string>::args("Current-space", "Current-time"),
      1.0 - windCurrentBalance));

  arma::mat X = calib.makeInitialParameters();

  GridFitter gf;
  gf.add(windTerm);
  gf.add(currentTerm);
  gf.solve(&X);

  double *x = X.memptr();
  std::cout << "Final calibration: " << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(boatData->magneticCompassOffset(x)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(boatData->windDirectionOffset(x)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(boatData->waterSpeedCoef(x)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(boatData->windSpeedCoef(x)) << std::endl;
}

} /* namespace sail */
