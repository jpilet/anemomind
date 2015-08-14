/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/nautical/MinCovCalib.h>

using namespace sail;

void calibrateForNavs(Array<Nav> navs) {

}

void evaluateForSimulation() {
  std::cout << "Prepare synthetic data..." << std::endl;
  NavalSimulation sim = getNavSimFractalWindOrientedLong();
  auto bd = sim.boatData(0);

  auto navs = bd.navs().sliceTo(100);

  std::cout << "Run the calibration..." << std::endl;
  MinCovCalib::Settings settings;
  Corrector<double> calibratedParameters = MinCovCalib::optimize(navs, settings);

  std::cout << "  With default parameters:   \n" << bd.evaluateNoCalibration();
  std::cout << "  With calibrated paramters: \n" << bd.evaluateFitness(calibratedParameters);
}

void standardBenchmark() {
  evaluateForSimulation();
}

int main(int argc, const char **argv) {
  standardBenchmark();
  return 0;
}


