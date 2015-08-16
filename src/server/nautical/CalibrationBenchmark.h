/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATIONBENCHMARK_H_
#define SERVER_NAUTICAL_CALIBRATIONBENCHMARK_H_

#include <functional>
#include <server/nautical/synthtest/NavalSimulation.h>

namespace sail {

typedef std::function<Corrector<double>(Array<Nav>)> Calibrator;



struct SynthResults {
  NavalSimulation::SimulatedCalibrationResults before, after;
};



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


struct RealDataResults {
 std::string datasetPath;
 Array<SplitResults> results;
};

struct CalibrationResults {
  Array<SynthResults> synthResults;
  Array<RealDataResults> realDataResults;

  void saveReportToFile(std::string filename);
};

CalibrationResults fullBenchmark(Calibrator calib);
CalibrationResults reducedBenchmark(Calibrator calib);

std::ostream &operator<<(std::ostream &s, CalibrationResults x);





} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIBRATIONBENCHMARK_H_ */
