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

  // The data part of the objective function
  DataCalib calib;
  calib.addBoatData(std::shared_ptr<BoatData>(new BoatData(&race, navs)));

  WindData windData(calib);
  CurrentData currentData(calib);

  std::cout << __LINE__ << std::endl;
  arma::sp_mat windRegSpace = kronWithSpEye(vcat(wgrid.makeFirstOrderReg(0),
                                   wgrid.makeFirstOrderReg(1)), 2);
  std::cout << __LINE__ << std::endl;
  arma::sp_mat windRegTime = kronWithSpEye(wgrid.makeFirstOrderReg(2), 2);
  std::cout << __LINE__ << std::endl;
  arma::sp_mat currentRegSpace = kronWithSpEye(vcat(cgrid.makeFirstOrderReg(0),
                                   cgrid.makeFirstOrderReg(1)), 2);
  std::cout << __LINE__ << std::endl;
  arma::sp_mat currentRegTime = kronWithSpEye(cgrid.makeFirstOrderReg(2), 2);
  std::cout << __LINE__ << std::endl;

  //arma::sp_mat windRegSpace2 = arma::kron(windRegSpace, arma::speye(2, 2));



  //GridFitter gf;
  //gf.add(std::shared_ptr<GridFit>(new GridFit()));
}

} /* namespace sail */
