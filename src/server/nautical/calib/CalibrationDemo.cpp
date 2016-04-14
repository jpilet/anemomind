/*
 *  Created on: 2014-11-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/AutoCalib.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/common/ArgMap.h>
#include <server/common/string.h>
#include <server/nautical/synthtest/NavalSimulation.h>
#include <iostream>


using namespace sail;

namespace {

  // A preconfigured example
  void psaros33FragmentDemo() {
    Poco::Path p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets/psaros33_Banque_Sturdza").get();
    NavCollection navs = scanNmeaFolderWithSimulator(p, Nav::debuggingBoatId()).slice(55895, 79143);
    FilteredNavData filtered(navs, 500);
    AutoCalib calib;
    AutoCalib::Results results = calib.calibrate(filtered);
    results.disp();
  }

  void synthDemo() {

    auto sim = makeNavSimUpwindDownwindLong();
    auto boatData = sim.boatData(0);
    auto initialErrors = boatData.evaluateNoCalibration();

    NavCollection navs = boatData.navs();
    FilteredNavData filtered(navs, 12.0);
    AutoCalib calib;
    AutoCalib::Results results = calib.calibrate(filtered);

    int count = navs.size();
    Array<HorizontalMotion<double> > estWind(count), estCurrent(count);
    for (int i = 0; i < count; i++) {
      auto c = results.corrector().correct(navs[i]);
      estWind[i] = c.trueWindOverGround();
      estCurrent[i] = c.trueCurrentOverGround();
    }

    auto finalErrors = boatData.evaluateFitness(estWind, estCurrent);
    results.disp(&(std::cout));
    std::cout << EXPR_AND_VAL_AS_STRING(initialErrors) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(finalErrors) << std::endl;
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  amap.registerOption("--psaros33-fragment-demo",
      "Run a demo on a fragment of the Psaros 33 recordings")
      .callback(psaros33FragmentDemo);
  amap.registerOption("--synth-demo", "Run a demo on synthetic data")
      .callback(synthDemo);

  if (amap.parse(argc, argv) == ArgMap::Error) {
    return -1;
  }

  /* TODO: (In another PR?)
   *   - Parse arguments so that we can try it on many different datasets.
   *   - Cross-validation
   *   - Artificially corrupt measurements and see if those corruptions are corrected for.
   */

  return 0;
}


