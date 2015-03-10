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

using namespace sail;

namespace {
  void calibrateAndMakeReport(Poco::Path p) {
    auto id = Nav::debuggingBoatId();
    Calibrator calib;

    std::cout << "EVALUATION ON DATASET " << p.toString() << std::endl;

    Corrector<double> defaultParameters;

    Array<Nav> navs = scanNmeaFolder(p, id);

    Corrector<double> calibratedParameters = calibrateFull(&calib, navs, id);



    std::cout << "\n\n" << std::endl;
  }
}

int main(int argc, const char **argv) {
  auto paths = getRealDatasetPaths();

  for (auto p : paths) {
    calibrateAndMakeReport(p);
  }
}

