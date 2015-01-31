/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/DecorrCalib.h>
#include <server/nautical/synthtest/NavalSimulation.h>
#include <server/common/logging.h>

using namespace sail;

int main(int argc, const char **argv) {
  NavalSimulation navsim = makeNavSimConstantFlow();//makeNavSimFractalWindOriented();
  auto boatData = navsim.boatData(0);

  double lambda = 0.5;
  FilteredNavData fdata(boatData.navs(), lambda, FilteredNavData::SIGNAL);

  LOG(INFO) << "Calibrate...";
  DecorrCalib calib;
  auto results = calib.calibrate(fdata);
  LOG(INFO) << "Done.";

  std::cout << "Before calibreation with default values: " << boatData.evaluateFitness(Corrector<double>());

  std::cout << "After calibration with optimal values:  " <<
            boatData.evaluateFitness(results.corrector);



  return 0;
}


