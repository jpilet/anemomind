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

SynthResults evaluateForSimulation(Calibrator calib) {
  std::cout << "Prepare synthetic data..." << std::endl;
  NavalSimulation sim = getNavSimFractalWindOrientedLong();
  auto bd = sim.boatData(0);

  //auto navs = bd.navs().sliceTo(10000);
  auto navs = bd.navs();

  std::cout << "Run the calibration..." << std::endl;
  Corrector<double> calibratedParameters = calib(navs);
  return SynthResults{
    bd.evaluateNoCalibration(),
    bd.evaluateFitness(calibratedParameters)
  };
}

SplitResults evaluateForSplit(Calibrator calib, Array<Nav> navs) {
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
    s << "Corrector for first  half: " << r.a.toArray() << std::endl;
    s << "Corrector for second half: " << r.b.toArray() << std::endl;
    s << "Cross validation errors:\n";
    s << r.errors;
    s << "\n";
  }
  return s;
}

RealDataResults evaluateForRealData(Calibrator calib, std::string datasetPath,
    Arrayi optionalInds = Arrayi()) {
  ENTERSCOPE(stringFormat("Evaluating dataset %s", datasetPath.c_str()));
  auto navs = scanNmeaFolder(datasetPath, Nav::debuggingBoatId(), nullptr)
          .slice(hasAllData);
  auto splits = splitNavsByDuration(navs, Duration<double>::hours(1.0));
  if (!optionalInds.empty()) {
    splits = splits.slice(optionalInds);
  }
  SCOPEDMESSAGE(INFO, stringFormat("  Loaded %d navs.", navs.size()));
  SCOPEDMESSAGE(INFO, stringFormat("  Split into %d groups", splits.size()));
  for (int i = 0; i < splits.size(); i++) {
    auto split = splits[i];
    double dur = (split.last().time() - split.first().time()).seconds();
    double period = dur/split.size();
    SCOPEDMESSAGE(INFO, stringFormat("  Split %d/%d has %d navs and average period of %.3g seconds.",
      i+1, splits.size(), split.size(), period));
  }
  return RealDataResults{
    datasetPath,
    splits.map<SplitResults>([&](Array<Nav> navs) {return evaluateForSplit(calib, navs);})
  };
}

CalibrationResults fullBenchmark(Calibrator calib) {
  auto synthResults = evaluateForSimulation(calib);
  auto exocet = evaluateForRealData(calib, getDatasetPath("exocet").toString());
  auto ps33 = evaluateForRealData(calib, getDatasetPath("psaros33_Banque_Sturdza").toString());
  auto irene = evaluateForRealData(calib, getDatasetPath("Irene").toString());
  return CalibrationResults{
    Array<SynthResults>{synthResults},
    Array<RealDataResults>{exocet, ps33, irene}
  };
}

CalibrationResults reducedBenchmark(Calibrator calib) {
  auto ps33 = evaluateForRealData(calib,
      getDatasetPath("psaros33_Banque_Sturdza").toString(),
      Arrayi{0});
  auto synthResults = evaluateForSimulation(calib);
  return CalibrationResults{
    Array<SynthResults>{synthResults},
    Array<RealDataResults>{ps33}
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




} /* namespace sail */
