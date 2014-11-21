/*
 *  Created on: 2014-11-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/AutoCalib.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/common/DataSplits.h>
#include <server/common/ScopedLog.h>


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

  Arrayb makeSubset(int length, int subsetSize,
      std::default_random_engine &e) {
      Arrayb subset(length);
      std::uniform_int_distribution<int> distrib(0, length);
      for (int i = 0; i < length; i++) {
        subset[i] = distrib(e) < subsetSize;
      }
      return subset;
  }

  Array<Arrayb> makeSubsets(int setCount,
      int length, int subsetSize, std::default_random_engine &e) {
    Array<Arrayb> dst(setCount);
    for (int i = 0; i < setCount; i++) {
      dst[i] = makeSubset(length, subsetSize, e);
    }
    return dst;
  }

  void ex1() {
    ScopedLog::setDepthLimit(5);
    std::default_random_engine e;
    Poco::Path p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets/psaros33_Banque_Sturdza").get();
    Array<Nav> navs = scanNmeaFolder(p, Nav::debuggingBoatId()).slice(55895, 79143);
    FilteredNavData filtered(navs, 500);
    AutoCalib calib;
    Arrayd times = filtered.makeCenteredX();
    Array<Arrayb> subset = makeSubsets(4, times.size(), 100, e);
    AutoCalib::Results results = calib.calibrateAutotuneGame(filtered, times, subset);
    results.disp();
  }

}

int main(int argc, const char **argv) {
  //runExtraAutoCalibTests();

  ex1();

  /* TODO: (In another PR?)
   *   - Parse arguments so that we can try it on many different datasets.
   *   - Cross-validation
   *   - Artificially corrupt measurements and see if those corruptions are corrected for.
   */

  return 0;
}


