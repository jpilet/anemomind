/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/nautical/Calibrator.h>
#include <server/common/string.h>


namespace {
  using namespace sail;

  NavalSimulation::SimulatedCalibrationResults
    evaluateCalibration(NavalSimulation::BoatData boatData,
      const Calibrator &c) {
    Array<Nav> navsWithTrueWind = boatData.navs();
    c.simulate(&navsWithTrueWind);
    Array<HorizontalMotion<double> > trueWind =
        navsWithTrueWind.map<HorizontalMotion<double> >([=](const Nav &x) {
          return x.trueWind();
    });
    return boatData.evaluateFitness(trueWind, Array<HorizontalMotion<double> >());
  }

  void evaluate() {
    std::cout << "Synthesize the dataset..." << std::endl;
    auto sim = getNavSimFractalWindOriented();
    std::cout << "Done synthesis." << std::endl;

    auto boatData = sim.boatData(0);

    boatData.plot();

    Array<Nav> navs = boatData.navs();
    WindOrientedGrammarSettings gs;
    WindOrientedGrammar grammar(gs);
    auto tree = grammar.parse(navs);

    Calibrator calib(grammar);
    calib.setVerbose();


    auto before = evaluateCalibration(boatData, calib);
    assert(calib.calibrate(navs, tree, Nav::debuggingBoatId()));
    auto after = evaluateCalibration(boatData, calib);

    std::cout << "Number of maneuvers used: " << calib.maneuverCount() << std::endl;
    std::cout << "Before calibration with default values: " << before << std::endl;
    std::cout << "After calibration with optimal values:  " << after << std::endl;
  }
}

int main() {
  evaluate();
  return 0;
}
