/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */


#include <server/nautical/CalibrationBenchmark.h>
#include <server/nautical/MinCovCalib.h>
#include <server/nautical/Calibrator.h>
#include <server/common/ArgMap.h>

using namespace sail;
using namespace sail::Benchmark;

// This is a class to allow for testing of maneuverbased calibration.
class ManeuverbasedCorrectorFunction : public CorrectorFunction {
 public:
  ManeuverbasedCorrectorFunction(std::shared_ptr<Calibrator> calib) : _calib(calib) {}

  Array<CalibratedNav<double> > operator()(const Array<Nav> &navs) const {
    auto correctedNavs = navs.dup();
    _calib->simulate(&correctedNavs);
    int n = navs.size();
    Array<CalibratedNav<double> > dst(n);
    Corrector<double> corr;
    for (int i = 0; i < n; i++) {
      auto c = correctedNavs[i];
      dst[i] = corr.correct(c);
      dst[i].trueWindOverGround.set(c.trueWindOverGround());
    }
    return dst;
  }

  std::string toString() const {
    return "Maneuverbased calibration";
  }
 private:
  std::shared_ptr<Calibrator> _calib;
};

std::shared_ptr<CorrectorFunction> calibrateManeuvers(Array<Nav> navs, bool full) {
  WindOrientedGrammarSettings gs;
  WindOrientedGrammar grammar(gs);
  auto tree = grammar.parse(navs);

  std::shared_ptr<Calibrator> calib(new Calibrator(grammar));
  calib->setVerbose();

  if (full) {
    return std::shared_ptr<CorrectorFunction>(
        new CorrectorObject(calibrateFull(calib.get(), navs, tree, Nav::debuggingBoatId())));
  } else {
    calib->calibrate(navs, tree, Nav::debuggingBoatId());
    return std::shared_ptr<CorrectorFunction>(new ManeuverbasedCorrectorFunction(calib));
  }
}

std::shared_ptr<CorrectorFunction> calibrateManeuversWind(Array<Nav> navs) {
  return calibrateManeuvers(navs, false);
}

std::shared_ptr<CorrectorFunction> calibrateManeuversFull(Array<Nav> navs) {
  return calibrateManeuvers(navs, true);
}

std::shared_ptr<CorrectorFunction> calibrateMinCov(Array<Nav> navs) {
  FilteredNavData data(navs, 0.5);
  return std::shared_ptr<CorrectorFunction>(
      new CorrectorObject(MinCovCalib::optimizeWindVsCurrent(data, MinCovCalib::Settings())));
}





// Whenever we add a new benchmark setup, add it here.
typedef std::map<std::string, std::function<CalibrationResults(CalibrationAlgorithm)> > TestMap;
TestMap makeTestMap() {
  TestMap testMap;
  testMap["full"] = fullBenchmark;
  testMap["reduced"] = reducedBenchmark;
  testMap["irene"] = longIreneBenchmark;
  return testMap;
}




// Whenever we want to test an algorithm, add it here.
typedef std::map<std::string, CalibrationAlgorithm> AlgoMap;
AlgoMap makeAlgoMap() {
  AlgoMap algoMap;
  algoMap["manfull"] = calibrateManeuversFull;
  algoMap["manwind"] = calibrateManeuversWind;
  algoMap["mincov"] = calibrateMinCov;
  return algoMap;
}












template <typename T>
std::string listKeysCommaSeparated(std::map<std::string, T> m) {
  std::string dst;
  for (auto k: m) {
    dst += k.first + ", ";
  }
  return dst.substr(0, dst.size()-2); // remove last comma.
}







int performBenchmark(
    TestMap &testMap, AlgoMap &algoMap,
    std::string testCode, std::string algo,
    std::string folder,
    std::string filename) {

  if (testMap.find(testCode) == testMap.end() && folder.empty()) {
    LOG(ERROR) << "No such test: '" << testCode << "'";
    return -1;
  }
  auto test = testMap[testCode];

  // But if a folder was provided, that will override the test
  if (!folder.empty()) {
    test = [&](CalibrationAlgorithm algorithm) {
      return folderBenchmark(folder, algorithm);
    };
  }

  if (algoMap.find(algo) == algoMap.end()) {
    LOG(ERROR) << "No such algorithm: '" << algo << "'";
    return -1;
  }

  auto results = test(algoMap[algo]);

  if (filename.empty()) {
    filename = "/tmp/calibtest_" + testCode + "_" + algo + ".txt";
  }
  results.saveReportToFile(filename);
  LOG(INFO) << "Results written to " << filename << std::endl;

  return 0;
}

int main(int argc, const char **argv) {
  auto algoMap = makeAlgoMap();
  auto testMap = makeTestMap();

  std::string testCode = "reduced";
  std::string algo = "mincov";
  std::string filename = "";
  std::string folder = "";

  ArgMap amap;
  amap.setHelpInfo(
      std::string("") +
      "Evaluation of calibration algorithms on real and synthetic data\n" +
      "Example usages: \n" +
      "  ./nautical_EvalCalib --test irene --algo manwind\n" +
      "  ./nautical_EvalCalib --test reduced --algo mincov\n" +
      "  ./nautical_EvalCalib --test reduced --algo mincov --output myspecialresults.txt\n\n"
  );
  amap.registerOption("--test", "What test to run (" + listKeysCommaSeparated(testMap) + ")")
    .store(&testCode);
  amap.registerOption("--algo", "What algorithm to test (" + listKeysCommaSeparated(algoMap) + ")")
    .store(&algo);
  amap.registerOption("--output", "Where the results should be saved")
    .store(&filename);
  amap.registerOption("--folder", "Instead of providing a test (using --test), read data from a folder")
    .store(&folder);

  auto status = amap.parse(argc, argv);
  switch (status) {
   case ArgMap::Done:
     return 0;
   case ArgMap::Continue:
     return performBenchmark(testMap, algoMap, testCode, algo, folder, filename);
   case ArgMap::Error:
     return -1;
  };
}



