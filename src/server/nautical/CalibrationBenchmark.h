/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATIONBENCHMARK_H_
#define SERVER_NAUTICAL_CALIBRATIONBENCHMARK_H_

#include <functional>
#include <server/nautical/synthtest/NavalSimulation.h>
#include <memory>

namespace sail {
namespace Benchmark {



// Results on synthetic data.
struct SynthResults {
  NavalSimulation::SimulatedCalibrationResults before, after;
};

// Results from crossvalidation on real data. How consistent the calibration is.
struct SplitResults {
  SplitResults(std::shared_ptr<CorrectorFunction> aCorr, std::shared_ptr<CorrectorFunction> bCorr,
      NavCollection n) : a(aCorr), b(bCorr),
      errors(compareCorrectors(*aCorr, *bCorr, n)),
      navs(n) {}
  SplitResults() {}

  std::shared_ptr<CorrectorFunction> a, b;
  NavCollection navs;
  WindCurrentErrors errors;
};


// Results from testing the algorithm on real recorded data
struct RealDataResults {
 std::string datasetPath;
 Array<SplitResults> results;
};

struct CalibrationResults {
  Array<SynthResults> synthResults;
  Array<RealDataResults> realDataResults;

  void saveReportToFile(std::string filename);
};

// We want to benchmark a calibration algorithm. A calibration algorithm
// should, given a bunch of Navs, produce a function that can map
// a Nav to a CalibratedNav<double>
typedef std::function<std::shared_ptr<CorrectorFunction>(NavCollection)> CalibrationAlgorithm;



/*
 *
 * A couple of standard tests that
 * we may want to try.
 *
 */


// Test the calibration algorithm on the synthetic data,
// and all the real datasets there are. Split the real data series into
// smaller chunks. Useful to check if an algorithm works across a range
// of situations.
CalibrationResults fullBenchmark(CalibrationAlgorithm calib);

// Test the calibration algorithm on a a small real dataset, and
// a small synthetic one. Useful when developing the algorithm.
CalibrationResults reducedBenchmark(CalibrationAlgorithm calib);

// Test the algorithm on the irene dataset, without splitting it.
// Useful for algorithms that require lots of data in order to work.
CalibrationResults longIreneBenchmark(CalibrationAlgorithm calib);

CalibrationResults folderBenchmark(std::string folder, CalibrationAlgorithm calib);

std::ostream &operator<<(std::ostream &s, CalibrationResults x);




}
} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIBRATIONBENCHMARK_H_ */
