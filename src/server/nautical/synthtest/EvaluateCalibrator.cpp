/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulation.h>
#include <server/nautical/Calibrator.h>
#include <server/common/string.h>


namespace {
  using namespace sail;

  NavalSimulation::FlowErrors evaluateCalibration(NavalSimulation::BoatData boatData,
      const Calibrator &c) {
    Array<Nav> navsWithTrueWind = boatData.navs();
    c.simulate(&navsWithTrueWind);
    Array<HorizontalMotion<double> > trueWind =
        navsWithTrueWind.map<HorizontalMotion<double> >([=](const Nav &x) {
          return x.trueWind();
    });
    return boatData.evaluateFitnessPerNav(trueWind, Array<HorizontalMotion<double> >());
  }

  void evaluate() {
    auto sim = makeNavSimUpwindDownwindLong();
    auto boatData = sim.boatData(0);
    Array<Nav> navs = boatData.navs();
    WindOrientedGrammarSettings gs;
    WindOrientedGrammar grammar(gs);
    auto tree = grammar.parse(navs);
    Calibrator calib(grammar);

    std::cout << "Before calibration with default values: " <<
        evaluateCalibration(boatData, calib);

    assert(calib.calibrate(navs, tree, Nav::debuggingBoatId()));

    std::cout << "After calibration with optimal values:  " <<
        evaluateCalibration(boatData, calib);
  }
}

int main() {
  evaluate();
  return 0;
}
