/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/common/EnvUtil.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/ArrayIO.h>
#include <server/nautical/CalibrationBenchmark.h>
#include <server/common/ScopedLog.h>
#include <fstream>

namespace sail {
namespace Benchmark {

bool hasAllData(const Nav &x) {
  return x.hasApparentWind() && x.hasMagHdg() && x.hasWatSpeed();
}

std::ostream &operator<<(std::ostream &s, SynthResults results) {
  s << "\n****** Results on synthetic data ******\n";
  s << "Before calibration:\n";
  s << results.before;
  s << "After calibration:\n";
  s << results.after;
  return s;
}


NavalSimulation::BoatData getStandardBoatData() {
  std::cout << "Prepare synthetic data..." << std::endl;
  NavalSimulation sim = getNavSimFractalWindOrientedLong();
  return sim.boatData(0);
}

SynthResults evaluateForSimulation(NavalSimulation::BoatData bd,
    CalibrationAlgorithm calib) {
  auto navs = bd.navs();

  std::cout << "Run the calibration..." << std::endl;
  auto calibratedParameters = calib(navs);
  return SynthResults{
    bd.evaluateNoCalibration(),
    bd.evaluateFitness(*calibratedParameters)
  };
}

SplitResults evaluateForSplit(CalibrationAlgorithm calib, Array<Nav> navs) {
  int middle = navs.middle();
  return SplitResults(
    calib(navs.sliceTo(middle)),
    calib(navs.sliceFrom(middle)),
    navs
  );
}

std::ostream &operator<<(std::ostream &s, RealDataResults x) {
  s << "\n\n****** RESULTS ON REAL DATASET " << x.datasetPath << "******\n";
  auto n = x.results.size();
  for (int i = 0; i < n; i++) {
    auto r = x.results[i];
    s << "Result for subset " << i+1 << "/" << n << " of " << r.navs.size() << " navs.\n";
    s << "Corrector for first  half: " << r.a->toString() << std::endl;
    s << "Corrector for second half: " << r.b->toString() << std::endl;
    s << "Cross validation errors:\n";
    s << r.errors;
    s << "\n";
  }
  return s;
}


Array<Nav> loadAndFilterDataset(std::string datasetPath) {
  return scanNmeaFolder(datasetPath, Nav::debuggingBoatId(), nullptr)
          .slice(hasAllData);
}

Array<Array<Nav> > splitRealData(Array<Nav> navs) {
  return splitNavsByDuration(navs, Duration<double>::hours(1.0));
}

RealDataResults evaluateForRealDataSplits(CalibrationAlgorithm algo,
    std::string dsPath, Arrayi subset = Arrayi()) {
  auto navs = splitRealData(loadAndFilterDataset(dsPath));
  if (!subset.empty()) {
    navs = navs.slice(subset);
  }
  auto splitResults = navs.map<SplitResults>([&](Array<Nav> navs) {
    return evaluateForSplit(algo, navs);
  });
  return RealDataResults{dsPath, splitResults};
}

RealDataResults evaluateWithoutSplitting(CalibrationAlgorithm algo, std::string dsPath) {
  auto navs = loadAndFilterDataset(dsPath);
  return RealDataResults{dsPath, Array<SplitResults>{evaluateForSplit(algo, navs)}};
}


CalibrationResults fullBenchmark(CalibrationAlgorithm calib) {
  auto synthResults = evaluateForSimulation(getStandardBoatData(), calib);
  auto exocet = evaluateForRealDataSplits(calib, getDatasetPath("exocet").toString());
  auto ps33 = evaluateForRealDataSplits(calib, getDatasetPath("psaros33_Banque_Sturdza").toString());
  auto irene = evaluateForRealDataSplits(calib, getDatasetPath("Irene").toString());
  return CalibrationResults{
    Array<SynthResults>{synthResults},
    Array<RealDataResults>{exocet, ps33, irene}
  };
}

CalibrationResults reducedBenchmark(CalibrationAlgorithm calib) {
  auto ps33 = evaluateForRealDataSplits(calib,
      getDatasetPath("psaros33_Banque_Sturdza").toString(),
      Arrayi{0});
  auto synthResults = evaluateForSimulation(getStandardBoatData(), calib);
  return CalibrationResults{
    Array<SynthResults>{synthResults},
    Array<RealDataResults>{ps33}
  };
}

CalibrationResults longIreneBenchmark(CalibrationAlgorithm calib) {
  return CalibrationResults{
    Array<SynthResults>(),
    Array<RealDataResults>{evaluateWithoutSplitting(calib, getDatasetPath("Irene").toString())}
  };
}

std::ostream &operator<<(std::ostream &s, CalibrationResults X) {
  s << "********************** CALIBRATION RESULTS\n";
  for (auto x: X.synthResults) {
    s << x;
  }
  for (auto x: X.realDataResults) {
    s << x;
  }
  return s;
}

void CalibrationResults::saveReportToFile(std::string filename) {
  std::ofstream file(filename);
  file << *this;
}



}
} /* namespace sail */
