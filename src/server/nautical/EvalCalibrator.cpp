/*
 *  Created on: 2015-03-09
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Code to evaluate the performance of the calibrator
 */

#include <server/nautical/RealCalibData.h>
#include <iostream>
#include <server/nautical/NavNmeaScan.h>
#include <server/nautical/Calibrator.h>
#include <iostream>
#include <server/nautical/synthtest/NavalSimulationPrecomp.h>

using namespace sail;

namespace {
  void calibrateAndMakeReport(Poco::Path p) {
    auto id = Nav::debuggingBoatId();
    Calibrator calib;

    std::cout << "EVALUATION ON DATASET " << p.toString() << std::endl;
    Corrector<double> defaultParameters;
    Array<Nav> navs = scanNmeaFolder(p, id);
    Corrector<double> calibratedParameters = calibrateFull(&calib, navs, id);

    std::cout << "  With default parameters:    \n" << computeErrors(&calib, defaultParameters) << std::endl;
    std::cout << "  With calibrated parameters: \n" << computeErrors(&calib, calibratedParameters) << std::endl;

    std::cout << "\n\n" << std::endl;
  }

  void calibrateOnSyntheticData() {
    std::cout << "\n\n============== EVALUTATION ON SYNTHETIC DATA" << std::endl;
    NavalSimulation sim = getNavSimFractalWindOrientedLong();
    auto bd = sim.boatData(0);

    Calibrator calib;
    auto navs = bd.navs();

    Corrector<double> calibratedParameters = calibrateFull(&calib, navs, Nav::debuggingBoatId());

    std::cout << "  With default parameters:   \n" << bd.evaluateNoCalibration();
    std::cout << "  With calibrated paramters: \n" << bd.evaluateFitness(calibratedParameters);

  }
}

int main(int argc, const char **argv) {
  {
    calibrateOnSyntheticData();
  }{
    auto paths = getRealDatasetPaths();
    int count = paths.size();
    for (int i = 0; i < count; i++) {
      std::cout << "\n\n============== DATASET " << i+1 << " OF " << count << std::endl;
      calibrateAndMakeReport(paths[i]);
    }
  }
}

