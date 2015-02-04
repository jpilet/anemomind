/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/nautical/FilteredNavData.h>
#include <server/nautical/SimpleCalibrator.h>
#include <server/common/string.h>

int main(int argc, const char **argv) {
  using namespace sail;

  NavalSimulation navsim = getNavSimFractalWindOriented(); //makeNavSimConstantFlow();//makeNavSimFractalWindOriented();
  auto boatData = navsim.boatData(0);

  double lambda = 0.5;
  FilteredNavData fdata(boatData.navs(), lambda, FilteredNavData::NONE);

  SimpleCalibrator calib;
  Corrector<double> corr = calib.calibrate(fdata);

  std::cout << EXPR_AND_VAL_AS_STRING(boatData.evaluateFitness(Corrector<double>())) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(boatData.evaluateFitness(corr)) << std::endl;

  std::cout << "Success" << std::endl;

  return 0;
}
