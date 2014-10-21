/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>

using namespace sail;

namespace {
  void fullCalib(Array<Nav> navs) {

  }

  void ex0() {
    Array<Nav> navs =
        scanNmeaFolder("/home/jonas/programmering/sailsmart/datasets/psaros33_Banque_Sturdza",
        Nav::debuggingBoatId());
    fullCalib(navs);
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--ex0", "Run a preconfigured example");
  if (!amap.parseAndHelp(argc, argv)) {
    return -1;
  }

  if (amap.optionProvided("--ex0")) {
    ex0();
  } else {
    fullCalib(getTestdataNavs(amap));
  }
  return 0;
}
