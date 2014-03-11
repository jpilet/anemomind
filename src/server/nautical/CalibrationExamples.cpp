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
#include <server/common/ScopedLog.h>

namespace sail {


namespace {
  class CalibSetup { // temp class with all the test data we need
   public:
    CalibSetup(int sampleCount);
    Array<Nav> navs;
    std::shared_ptr<LocalRace> race;
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
    std::shared_ptr<DriftModel> driftModel;
  };

  CalibSetup::CalibSetup(int sampleCount) {
    Array<Nav> allNavs = loadNavsFromText(Nav::AllNavsPath, false);
    Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs,
                                   Duration<double>::minutes(10).seconds());
    Array<Nav> navs_ = splitNavs.first();
    navs = navs_.slice(makeSparseInds(navs_.size(), sampleCount));
    double spaceStep = 1000; // metres
    double timeStep = Duration<double>::minutes(20).seconds();

    race = std::shared_ptr<LocalRace>(new LocalRace(navs, spaceStep, timeStep));
    wgrid = race->getWindGrid();
    cgrid = race->getCurrentGrid();

    driftModel = std::shared_ptr<DriftModel>(new SinusDriftAngle());
    boatData = std::shared_ptr<BoatData>(new BoatData(race.get(), navs, driftModel.get()));

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
  s.race->plotTrajectoryVectors(s.navs, toPlot, 10.0);
}



void calibEx(bool autoTune, double initialRegWeight) { // Try to optimize it
  CalibSetup s(60);
  WindData windData(s.calib);
  CurrentData currentData(s.calib);

  ScopedLog::setDepthLimit(3);

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
  int gridVertexCount = s.race->getWindGrid().getVertexCount();


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

  if (autoTune) {
    gf.solve(&X); // <-- May crash because the automatic parameter tuning diverges. Needs more work / research effort.
  } else {
    gf.solveFixedReg(&X);
  }

  double *x = X.memptr();
  std::cout << "Final calibration: " << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(s.boatData->magneticCompassOffset(x)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(s.boatData->windDirectionOffset(x)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(s.boatData->waterSpeedCoef(x)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(s.boatData->windSpeedCoef(x)) << std::endl;
}

void calibEx002() {
  //calibEx(true, 1.0e3);

  calibEx(false, 1.0e3); // Quite a lot of regularization => assume homogeneous wind/current
}


} /* namespace sail */
