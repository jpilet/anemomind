/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/nautical/MinCovCalib.h>
#include <server/common/EnvUtil.h>

using namespace sail;

void calibrateForNavs(Array<Nav> navs) {

}

struct SynthResults {
  NavalSimulation::SimulatedCalibrationResults before, after;
};

std::ostream &operator<<(std::ostream &s, SynthResults results) {
  s << "\n****** Results on synthetic data ******\n";
  s << "Before calibration:\n";
  s << results.before;
  s << "After calibration:\n";
  s << results.after;
  return s;
}

SynthResults evaluateForSimulation() {
  std::cout << "Prepare synthetic data..." << std::endl;
  NavalSimulation sim = getNavSimFractalWindOrientedLong();
  auto bd = sim.boatData(0);

  //auto navs = bd.navs().sliceTo(10000);
  auto navs = bd.navs();

  std::cout << "Run the calibration..." << std::endl;
  MinCovCalib::Settings settings;
  Corrector<double> calibratedParameters = MinCovCalib::optimize(navs, settings);
  return SynthResults{
    bd.evaluateNoCalibration(),
    bd.evaluateFitness(calibratedParameters)
  };
}

void evaluateForRealData(std::string datasetPath) {

}

void standardBenchmark() {
  auto synthResults = evaluateForSimulation();
  evaluateForRealData(getDatasetPath("exocet").toString());
  evaluateForRealData(getDatasetPath("psaros33_Banque_Sturdza").toString());
  evaluateForRealData(getDatasetPath("Irene").toString());

  std::cout << EXPR_AND_VAL_AS_STRING(synthResults) << std::endl;
}

int main(int argc, const char **argv) {
  standardBenchmark();
  return 0;
}


