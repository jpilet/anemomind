/*
 *  Created on: 2015-03-09
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Code to evaluate the performance of the calibrator
 */

#include <server/nautical/RealCalibData.h>
#include <iostream>

using namespace sail;

namespace {
  void calibrateAndMakeReport(Poco::Path p) {
    std::cout << "EVALUATION ON DATASET " << p.toString() << std::endl;

    std::cout << "\n\n" << std::endl;
  }
}

int main(int argc, const char **argv) {
  auto paths = getRealDatasetPaths();

  for (auto p : paths) {
    calibrateAndMakeReport(p);
  }
}

