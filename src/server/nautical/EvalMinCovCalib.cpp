/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/nautical/MinCovCalib.h>
#include <server/common/EnvUtil.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/ArrayIO.h>

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

struct SplitResults {
  SplitResults(Corrector<double> aCorr, Corrector<double> bCorr,
      Array<Nav> n) : a(aCorr), b(bCorr),
      errors(compareCorrectors(aCorr, bCorr, n)),
      navs(n) {}
  SplitResults() {}

  Corrector<double> a, b;
  Array<Nav> navs;
  WindCurrentErrors errors;
};

SplitResults evaluateForSplit(Array<Nav> navs) {
  int middle = navs.middle();
  MinCovCalib::Settings settings;
  return SplitResults(
    MinCovCalib::optimize(navs.sliceTo(middle), settings),
    MinCovCalib::optimize(navs.sliceFrom(middle), settings),
    navs
  );
}

struct RealDataResults {
 std::string datasetPath;
 Array<SplitResults> results;
};

std::ostream &operator<<(std::ostream &s, RealDataResults x) {
  s << "\n\n****** RESULTS ON REAL DATASET " << x.datasetPath << "******";
  auto n = x.results.size();
  for (int i = 0; i < n; i++) {
    auto r = x.results[i];
    s << "Result for subset " << i+1 << "/" << n << " of " << r.navs.size() << " navs.\n";
    s << "Corrector for first  half: " << r.a.toArray() << std::endl;
    s << "Corrector for second half: " << r.b.toArray() << std::endl;
    s << "Cross validation errors:\n";
    s << r.errors;
    s << "\n";
  }
  return s;
}

RealDataResults evaluateForRealData(std::string datasetPath) {
  ENTERSCOPE(stringFormat("Evaluating dataset %s", datasetPath.c_str()));
  auto navs = scanNmeaFolder(datasetPath, Nav::debuggingBoatId(), nullptr)
          .slice(MinCovCalib::hasAllData);
  auto splits = splitNavsByDuration(navs, Duration<double>::hours(1.0));
  SCOPEDMESSAGE(INFO, stringFormat("  Loaded %d navs.", navs.size()));
  SCOPEDMESSAGE(INFO, stringFormat("  Split into %d groups", splits.size()));
  return RealDataResults{
    datasetPath,
    splits.map<SplitResults>(evaluateForSplit)
  };
}

void standardBenchmark() {
  //auto synthResults = evaluateForSimulation();
  auto exocet = evaluateForRealData(getDatasetPath("exocet").toString());
  auto ps33 = evaluateForRealData(getDatasetPath("psaros33_Banque_Sturdza").toString());
  auto irene = evaluateForRealData(getDatasetPath("Irene").toString());

  std::cout << /*synthResults <<*/ exocet << ps33 << irene;
}

int main(int argc, const char **argv) {
  standardBenchmark();
  return 0;
}



