/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/DecorrCalib.h>
#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/common/logging.h>
#include <server/common/ArgMap.h>

using namespace sail;

int main(int argc, const char **argv) {
  std::default_random_engine e;
  ArgMap amap;
  amap.registerOption("--gen-correctors", "Generate code for random correctors");
  amap.registerOption("--run-old", "Run the old version of the algorithm, that doesn't work completely.");
  amap.registerOption("--run", "Run the current version of the algorithm, with random correctors.");

  auto argCode = amap.parse(argc, argv);
  if (argCode == ArgMap::Error) {
    return -1;
  } else if (argCode == ArgMap::Done) {
    return 0;
  }

  if (amap.optionProvided("--gen-correctors")) {
    makeRandomCorrectors(30, e);
  } else {

    NavalSimulation navsim = getNavSimFractalWindOrientedLong(); //makeNavSimConstantFlow();//makeNavSimFractalWindOriented();
    auto boatData = navsim.boatData(0);

    double lambda = 0.5;
    FilteredNavData fdata(boatData.navs(), lambda, FilteredNavData::SIGNAL);



    LOG(INFO) << "Calibrate...";
    DecorrCalib calib;

    DecorrCalib::Results results;
    if (amap.optionProvided("--run-old")) {
      results = calib.calibrate(fdata);
    } else {
      results = calib.calibrate(fdata, makeCorruptCorrectors().sliceTo(4));
    }
    LOG(INFO) << "Done.";

    std::cout << "Before calibreation with default values: " << boatData.evaluateFitness(Corrector<double>());
    std::cout << "After calibration with optimal values:  " <<
              boatData.evaluateFitness(results.corrector);
  }



  return 0;
}


