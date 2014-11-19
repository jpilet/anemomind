/*
 *  Created on: 2014-11-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/AutoCalib.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/common/DataSplits.h>


using namespace sail;

namespace {

  // A preconfigured example
  void ex0() {
    Poco::Path p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets/psaros33_Banque_Sturdza").get();
    Array<Nav> navs = scanNmeaFolder(p, Nav::debuggingBoatId()).slice(55895, 79143);
    FilteredNavData filtered(navs, 500);
    AutoCalib calib;
    AutoCalib::Results results = calib.calibrate(filtered);
    results.disp();
  }

  // A preconfigured example
  void ex1() {
    std::default_random_engine e;
    Poco::Path p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets/psaros33_Banque_Sturdza").get();
    Array<Nav> navs = scanNmeaFolder(p, Nav::debuggingBoatId()).slice(55895, 79143);
    FilteredNavData filtered(navs, 500);
    AutoCalib calib;
    Arrayb split = makeRandomSplit(filtered.size(), e);
    Arrayd times = filtered.makeCenteredX();
    AutoCalib::Results results = calib.calibrateAutotune(filtered, times, split);
    results.disp();
  }

}

int main(int argc, const char **argv) {
  runExtraAutoCalibTests();

  ex0();

  /* TODO: (In another PR?)
   *   - Parse arguments so that we can try it on many different datasets.
   *   - Cross-validation
   *   - Artificially corrupt measurements and see if those corruptions are corrected for.
   */

  return 0;
}


