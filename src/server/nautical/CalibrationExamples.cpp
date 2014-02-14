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
#include <server/common/logging.h>

namespace sail {


namespace {
  class CalibSetup { // temp class with all the test data we need
   public:
    CalibSetup(int sampleCount);
    Array<Nav> navs;
    LocalRace race;
    Grid3d wgrid;
    Grid3d cgrid;

    std::shared_ptr<BoatData> boatData;

    DataCalib calib;

    Array<Arrayb> windSplits;
    Array<Arrayb> currentSplits;

    arma::sp_mat Pwind;
    arma::sp_mat windRegSpace;
    arma::sp_mat windRegTime;

    arma::sp_mat Pcurrent;
    arma::sp_mat currentRegSpace;
    arma::sp_mat currentRegTime;
  };

  CalibSetup::CalibSetup(int sampleCount) {
    Array<Nav> allNavs = loadNavsFromText(Nav::AllNavsPath, false);
    Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs,
                                   Duration::minutes(10).getDurationSeconds());
    Array<Nav> navs_ = splitNavs.first();
    navs = navs_.slice(makeSparseInds(navs_.size(), sampleCount));
    double spaceStep = 1000; // metres
    double timeStep = Duration::minutes(20).getDurationSeconds();

    race = LocalRace(navs, spaceStep, timeStep);
    wgrid = race.getWindGrid();
    cgrid = race.getCurrentGrid();

    boatData = std::shared_ptr<BoatData>(new BoatData(&race, navs));

    calib.addBoatData(boatData);
    windSplits = makeRandomSplits(4, calib.windDataCount());
    currentSplits = makeRandomSplits(4, calib.currentDataCount());
    Pwind = kronWithSpEye(calib.makeP(wgrid), 2);
    windRegSpace = kronWithSpEye(vcat(wgrid.makeFirstOrderReg(0),
        wgrid.makeFirstOrderReg(1)), 2);
    windRegTime = kronWithSpEye(wgrid.makeFirstOrderReg(2), 2);

    Pcurrent = kronWithSpEye(calib.makeP(cgrid), 2);
    currentRegSpace = kronWithSpEye(vcat(cgrid.makeFirstOrderReg(0),
        cgrid.makeFirstOrderReg(1)), 2);
    currentRegTime = kronWithSpEye(cgrid.makeFirstOrderReg(2), 2);
  }
}


void calibEx001() {
  CalibSetup s(55);
  WindData windData(s.calib);
  CurrentData currentData(s.calib);

  arma::mat X = s.calib.makeInitialParameters();

  Arrayd W(windData.outDims());
  windData.eval(X.memptr(), W.getData());

  Arrayd C(currentData.outDims());
  currentData.eval(X.memptr(), C.getData());


  Arrayd toPlot = C;
  s.race.plotTrajectoryVectors(s.navs, toPlot, 10.0);
}



void calibEx002() { // Try to optimize it
  CalibSetup s(60);
  WindData windData(s.calib);
  CurrentData currentData(s.calib);

  ScopedLog::setDepthLimit(3);

  const double initialRegWeight = 0.1;
  const int splitCount = 2;

  double windCurrentBalance = 0.5;
  std::shared_ptr<GridFit> windTerm(new GridFit(s.Pwind, &windData,
      Array<arma::sp_mat>::args(s.windRegSpace, s.windRegTime), s.windSplits,
      Arrayd::args(initialRegWeight, initialRegWeight),
      Array<std::string>::args("Wind-space", "Wind-time"),
      windCurrentBalance));
  std::shared_ptr<GridFit> currentTerm(new GridFit(s.Pcurrent, &currentData,
      Array<arma::sp_mat>::args(s.currentRegSpace, s.currentRegTime), s.currentSplits,
      Arrayd::args(initialRegWeight, initialRegWeight),
      Array<std::string>::args("Current-space", "Current-time"),
      1.0 - windCurrentBalance));

  arma::mat X = s.calib.makeInitialParameters();
  int gridVertexCount = s.race.getWindGrid().getVertexCount();


  GridFitter gf;
  gf.setPretuneWeightsIters(3);
  gf.settings.verbosity = 3;
  gf.add(windTerm);
  gf.add(currentTerm);

  std::cout << "Solving for " << s.wgrid.getVertexCount() << " wind grid 2d vertices and \n";
  std::cout << "  for " << s.cgrid.getVertexCount() << " current grid 2d vertices.\n";
  std::cout << "Number of calibration parameters: " << X.n_elem << "\n";
  std::cout << "Number of wind obs: " << windTerm->getData().outDims() << "\n";
  std::cout << "Number of current obs: " << currentTerm->getData().outDims() << "\n";


  gf.solve(&X);

  double *x = X.memptr();
  std::cout << "Final calibration: " << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(s.boatData->magneticCompassOffset(x)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(s.boatData->windDirectionOffset(x)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(s.boatData->waterSpeedCoef(x)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(s.boatData->windSpeedCoef(x)) << std::endl;
}

} /* namespace sail */
