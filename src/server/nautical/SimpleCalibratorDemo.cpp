/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/nautical/FilteredNavData.h>
#include <server/nautical/SimpleCalibrator.h>

int main(int argc, const char **argv) {
  using namespace sail;

  NavalSimulation navsim = getNavSimFractalWindOriented(); //makeNavSimConstantFlow();//makeNavSimFractalWindOriented();
  auto boatData = navsim.boatData(0);

  double lambda = 0.5;
  FilteredNavData fdata(boatData.navs(), lambda, FilteredNavData::NONE);

  SimpleCalibrator calib;
  Corrector<double> corr = calib.calibrate(fdata);

  return 0;
}
