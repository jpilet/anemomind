/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */


#include <server/nautical/CalibrationBenchmark.h>
#include <server/nautical/MinCovCalib.h>
#include <server/nautical/Calibrator.h>

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
      dst[i].trueWind.set(c.trueWind());
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

std::shared_ptr<CorrectorFunction> performCalibration(Array<Nav> navs) {
  FilteredNavData data(navs, 0.5);
  return std::shared_ptr<CorrectorFunction>(
      new CorrectorObject(MinCovCalib::optimizeWindVsCurrent(data, MinCovCalib::Settings())));
}

int main(int argc, const char **argv) {
  std::cout << reducedBenchmark(performCalibration);
  return 0;
}



