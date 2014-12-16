/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulation.h>
#include <server/nautical/Calibrator.h>
#include <server/common/string.h>


namespace {
  using namespace sail;

  void evaluate() {
    auto sim = makeNavSimUpwindDownwindLong();
    auto boatData = sim.boatData(0);
    Array<Nav> navs = boatData.navs();
    WindOrientedGrammarSettings gs;
    WindOrientedGrammar grammar(gs);
    auto tree = grammar.parse(navs);
    bool calibrate(const Array<Nav>& navs,
                   std::shared_ptr<HTree> tree,
                   Nav::Id boatId);
    Calibrator calib(grammar);
    assert(calib.calibrate(navs, tree, Nav::debuggingBoatId()));

    Array<Nav> navsWithTrueWind = navs.dup();
    calib.simulate(&navsWithTrueWind);
    Array<HorizontalMotion<double> > trueWind =
        navsWithTrueWind.map<HorizontalMotion<double> >([=](const Nav &x) {
          return x.trueWind();
    });
    auto fitness = boatData.evaluateFitnessPerNav(trueWind,
        Array<HorizontalMotion<double> >());
    std::cout << EXPR_AND_VAL_AS_STRING(fitness) << std::endl;
  }
}

int main() {
  evaluate();
  return 0;
}
